#include <zec_ext/IO/TcpClient.hpp>
#include <cstring>
#include "./_Local.hpp"

#include <iostream>

ZEC_NS
{

    bool xTcpClient::Init(xIoContext * IoContextPtr, const char * Ip, uint64_t Port, iListener * ListenerPtr)
    {
        xNetAddress Address = xNetAddress::Make(Ip);
        return Init(IoContextPtr, Address, Port, ListenerPtr);
    }

    bool xTcpClient::Init(xIoContext * IoContextPtr, const xNetAddress & Address, uint64_t Port, iListener * ListenerPtr)
    {
        assert(IoContextPtr);
        assert(ListenerPtr);
        assert(Address);
        assert(Port);
        assert(_State == eUnspecified);

        assert(!_IoContextPtr);
        _IoContextPtr = IoContextPtr;
        _ListenerPtr = ListenerPtr;
        _ReadDataSize = 0;
        _State = eInited;

        auto SocketPtr = NativeTcpSocketHolderRef(Native()).CreateValue(*IOUtil::Native(_IoContextPtr));
        SocketPtr->async_connect(MakeAddress(Address, Port), [this](const xAsioError & Error) {
            if (Error) {
                if (_State != eShuttingDown) {
                    _ListenerPtr->OnError(this);
                    _State = eShuttingDown;
                }
                return;
            }
            if (!_ListenerPtr->OnConnected(this)) {
                _State = eShuttingDown;
                return;
            }
            _State = eConnected;
            if (_WriteDataSize) {
                DoFlush();
            }
            DoRead();
        });
        return (_State == eInited);
    }

    void xTcpClient::Clean()
    {
        assert(_State != eUnspecified);
        NativeTcpSocketHolderRef(Native()).Destroy();
        // clear read buffer:
        _ReadDataSize = 0;
        // clear write buffers:
        while(auto WriteBufferPtr = _WritePacketBufferQueue.Pop()) {
            DeleteWriteBuffer(WriteBufferPtr);
        }
        _WriteDataSize = 0;
        Reset(_IoContextPtr);
        _State = eUnspecified;
    }

    void xTcpClient::DoRead()
    {
        auto SocketPtr = NativeTcpSocket(Native());
        size_t BufferSize = MaxPacketPayloadSize - _ReadDataSize;
        assert(BufferSize);
        SocketPtr->async_read_some(xAsioMutableBuffer{_ReadBuffer + _ReadDataSize, BufferSize}, [this, SocketPtr](const xAsioError & Error, size_t TransferedSize) {
            if (Error) {
                if (Error == asio::error::eof) {
                    _ListenerPtr->OnPeerClose(this);
                    return;
                }
                if (_State != eShuttingDown) {
                    _ListenerPtr->OnError(this);
                    _State = eShuttingDown;
                }
                return;
            }
            _ReadDataSize += TransferedSize;
            assert(_ReadDataSize <= MaxPacketSize);
            size_t ConsumedDataSize = _ListenerPtr->OnReceiveData(this, _ReadBuffer, _ReadDataSize);
            if (InvalidDataSize == ConsumedDataSize) {
                _State = eShuttingDown;
                return;
            }
            if (ConsumedDataSize && (_ReadDataSize -= ConsumedDataSize)) {
                memmove(_ReadBuffer, _ReadBuffer + ConsumedDataSize, _ReadDataSize);
            }
            DoRead();
        });
    }

    void xTcpClient::DoFlush()
    {
        auto SocketPtr = NativeTcpSocket(Native());
        auto BufferPtr = _WritePacketBufferQueue.Peek();
        assert (BufferPtr);
        SocketPtr->async_write_some(xAsioConstBuffer{BufferPtr->Buffer, BufferPtr->DataSize}, [this, BufferPtr](const xAsioError & Error, size_t TransferedBytes) {
            assert(BufferPtr == _WritePacketBufferQueue.Peek());
            assert(TransferedBytes <= BufferPtr->DataSize);
            if (Error) {
                if (_State != eShuttingDown) {
                    _ListenerPtr->OnError(this);
                    _State = eShuttingDown;
                }
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
                DoFlush();
            }
        });
    }

    size_t xTcpClient::PostData(const void * DataPtr_, size_t DataSize)
    {
        assert(DataSize);
        assert(_State != eShuttingDown);

        bool ExecWriteCall = (!_WriteDataSize && _State == eConnected);
        auto DataPtr = (const ubyte *)DataPtr_;
        auto PostSize = _WritePacketBufferQueue.Push(DataPtr, DataSize);
        auto RemainedDataSize = DataSize - PostSize;
        while(RemainedDataSize) {
            DataPtr += PostSize;
            auto BufferPtr = NewWriteBuffer();
            if (!BufferPtr) {
                if (DataSize == RemainedDataSize) {
                    return 0;
                }
                break;
            }
            _WritePacketBufferQueue.Push(BufferPtr);
            RemainedDataSize -= (PostSize = BufferPtr->Push(DataPtr, RemainedDataSize));
        }
        size_t TotalPostedSize = DataSize - RemainedDataSize;
        _WriteDataSize += TotalPostedSize;
        if (ExecWriteCall) {
            DoFlush();
        }
        return TotalPostedSize;
    }

}
