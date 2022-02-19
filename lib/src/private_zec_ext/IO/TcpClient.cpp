#include <zec_ext/IO/TcpClient.hpp>
#include <cstring>
#include "./_Local.hpp"

#include <iostream>

ZEC_NS
{

    bool xTcpClient::Init(xIoContext * IoContextPtr, const char * Ip, uint64_t Port, iListener * ListenerPtr)
    {
        xNetAddress Address = xNetAddress::Make(Ip);
        if (!Address || !Port) {
            return false;
        }
        return Init(IoContextPtr, Address, Port, ListenerPtr);
    }

    bool xTcpClient::Init(xIoContext * IoContextPtr, const xNetAddress & Address, uint64_t Port, iListener * ListenerPtr)
    {
        assert(IoContextPtr);
        assert(ListenerPtr);
        assert(_State == eUnspecified);

        assert(!_IoContextPtr);
        _IoContextPtr = IoContextPtr;
        _ServerAddress = Address;
        _ServerPort = Port;
        _ListenerPtr = ListenerPtr;
        _ReadDataSize = 0;
        _State = eInited;

        auto SocketPtr = NativeTcpSocketHolderRef(Native()).CreateValue(*IOUtil::Native(_IoContextPtr));
        SocketPtr->async_connect(MakeAddress(Address, Port), [this](const xAsioError & Error) {
            if (Error) {
                ErrorClose();
                return;
            }
            if (!_ListenerPtr->OnConnected(this)) {
                DoClose();
                return;
            }
            _State = eConnected;
            if (_WriteDataSize) {
                DoWrite();
            }
            DoRead();
        });
        return (_State == eInited);
    }

    void xTcpClient::Clean()
    {
        assert(_State != eUnspecified);
        if (_State < eClosing) {
            assert(_State != eClosing || "Do cleanup during callback is forbidden");
            // explicitly close socket to avoid later callbacks reentrance;
            DoClose();
        }
        NativeTcpSocketHolderRef(Native()).Destroy();
        Reset(_IoContextPtr);
        Reset(_ServerAddress);
        Reset(_ServerPort);
        _State = eUnspecified;
    }

    void xTcpClient::DoRead()
    {
        auto SocketPtr = NativeTcpSocket(Native());
        size_t BufferSize = MaxPacketPayloadSize - _ReadDataSize;
        assert(BufferSize);
        SocketPtr->async_read_some(xAsioBuffer{_ReadBuffer + _ReadDataSize, BufferSize}, [this, SocketPtr](const xAsioError & Error, size_t TransferedSize) {
            if (Error) {
                if (Error == asio::error::eof) {
                    _ListenerPtr->OnPeerClose(this);
                    return;
                }
                ErrorClose();
                return;
            }
            _ReadDataSize += TransferedSize;
            assert(_ReadDataSize <= MaxPacketSize);
            size_t ConsumedDataSize = _ListenerPtr->OnReceiveData(this, _ReadBuffer, _ReadDataSize);
            if (InvalidDataSize == ConsumedDataSize) {
                DoClose();
                return;
            }
            if (ConsumedDataSize && (_ReadDataSize -= ConsumedDataSize)) {
                memmove(_ReadBuffer, _ReadBuffer + ConsumedDataSize, _ReadDataSize);
            }
            DoRead();
        });
    }

    void xTcpClient::DoWrite()
    {
        auto SocketPtr = NativeTcpSocket(Native());
        auto BufferPtr = _WritePacketBufferQueue.Peek();
        assert (BufferPtr);
        SocketPtr->async_write_some(xAsioBuffer{BufferPtr->Buffer, BufferPtr->DataSize}, [this, BufferPtr](const xAsioError & Error, size_t TransferedBytes) {
            assert(BufferPtr == _WritePacketBufferQueue.Peek());
            assert(TransferedBytes <= BufferPtr->DataSize);
            if (Error) {
                ErrorClose();
                return;
            }
            if (auto RemainedDataSize = BufferPtr->DataSize - TransferedBytes) {
                memcpy(BufferPtr->Buffer, BufferPtr->Buffer + TransferedBytes, RemainedDataSize);
                BufferPtr->DataSize = RemainedDataSize;
            } else {
                _WritePacketBufferQueue.RemoveFront();
                DeleteWriteBuffer(BufferPtr);
            }
            if (_WriteDataSize -= TransferedBytes) {
                DoWrite();
            }
        });
    }

    size_t xTcpClient::PostData(const void * DataPtr_, size_t DataSize)
    {
        assert(DataSize);
        if (_State == eClosed) {
            return DataSize;
        }
        bool ExecWriteCall = (_State == eConnected && !_WriteDataSize);
        auto DataPtr = (const ubyte *)DataPtr_;
        auto PostedSize = _WritePacketBufferQueue.Push(DataPtr, DataSize);
        auto RemainedDataSize = DataSize - PostedSize;
        while(RemainedDataSize) {
            DataPtr += PostedSize;
            auto BufferPtr = NewWriteBuffer();
            if (!BufferPtr) {
                if (DataSize == RemainedDataSize) {
                    return DataSize;
                }
                break;
            }
            RemainedDataSize -= (PostedSize = BufferPtr->Push(DataPtr, RemainedDataSize));
            _WritePacketBufferQueue.Push(BufferPtr);
        }
        _WriteDataSize += (DataSize - RemainedDataSize);
        if (ExecWriteCall) {
            DoWrite();
        }
        return RemainedDataSize;
    }

    void xTcpClient::DoClose()
    {
        _State = eClosing;
        auto SocketPtr = NativeTcpSocket(Native());
        xAsioError Error;
        SocketPtr->close(Error);
        // clear read buffer:
        _ReadDataSize = 0;
        // clear write buffers:
        while(auto WriteBufferPtr = _WritePacketBufferQueue.Pop()) {
            DeleteWriteBuffer(WriteBufferPtr);
        }
        _WriteDataSize = 0;
        _State = eClosed;
    }

}
