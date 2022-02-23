#include <zec_ext/IO/TcpConnection.hpp>
#include <cstring>
#include "./_Local.hpp"

#include <iostream>

ZEC_NS
{

    bool xTcpConnection::Init(xNativeHandle NativeHandle, iListener * ListenerPtr)
    {
        _ListenerPtr = ListenerPtr;
        _Native.CreateValueAs<xNativeTcpSocket>(std::move(*(xNativeTcpSocket*)NativeHandle.ObjectPtr));
        return true;
    }

    bool xTcpConnection::Init(xIoContext * IoContextPtr, const char * Ip, uint64_t Port, iListener * ListenerPtr)
    {
        xNetAddress Address = xNetAddress::Make(Ip);
        return Init(IoContextPtr, Address, Port, ListenerPtr);
    }

    bool xTcpConnection::Init(xIoContext * IoContextPtr, const xNetAddress & Address, uint64_t Port, iListener * ListenerPtr)
    {
        assert(IoContextPtr);
        assert(ListenerPtr);
        assert(Address);
        assert(Port);

        assert(!_ReadDataSize);
        assert(!_WriteDataSize);

        _ListenerPtr = ListenerPtr;
        auto & Socket = _Native.CreateValueAs<xNativeTcpSocket>(*IOUtil::Native(IoContextPtr));
        Socket.async_connect(MakeEndpoint(Address, Port), [this](const xAsioError & Error) {
            if (Error) {
                OnError();
                return;
            }
            OnConnected();
        });
        return true;
    }

    void xTcpConnection::Clean()
    {
        // Set error true to prevent callbacks with "error caused by operation aborted"
        _Error = true;
        _Native.DestroyAs<xNativeTcpSocket>();
        _ListenerPtr = nullptr;

        // clear read buffer:
        _ReadDataSize = 0;
        // clear write buffers:
        while(auto WriteBufferPtr = _WritePacketBufferQueue.Pop()) {
            DeleteWriteBuffer(WriteBufferPtr);
        }
        _WriteDataSize = 0;
        _Connected = false;
        _Error = false;
    }

    void xTcpConnection::OnError()
    {
        if (_Error) {
            return;
        }
        _Error = true;
        _Connected = false;
        _ListenerPtr->OnError(this);
    }

    void xTcpConnection::OnConnected()
    {
        _ListenerPtr->OnConnected(this);
        _Connected = true;
        DoRead();
        if (_WriteDataSize) {
            DoFlush();
        }
    }

    void xTcpConnection::DoRead()
    {
        auto & Socket = _Native.As<xNativeTcpSocket>();
        size_t BufferSize = MaxPacketPayloadSize - _ReadDataSize;
        assert(BufferSize);
        Socket.async_read_some(xAsioMutableBuffer{_ReadBuffer + _ReadDataSize, BufferSize}, [this](const xAsioError & Error, size_t TransferedSize) {
            if (Error) {
                if (Error == asio::error::eof) {
                    _Connected = false;
                    _ListenerPtr->OnPeerClose(this);
                    return;
                }
                OnError();
                return;
            }
            _ReadDataSize += TransferedSize;
            assert(_ReadDataSize <= MaxPacketSize);
            size_t ConsumedDataSize = _ListenerPtr->OnReceiveData(this, _ReadBuffer, _ReadDataSize);
            if (ConsumedDataSize && (_ReadDataSize -= ConsumedDataSize)) {
                memmove(_ReadBuffer, _ReadBuffer + ConsumedDataSize, _ReadDataSize);
            }
            DoRead();
        });
    }

    void xTcpConnection::DoFlush()
    {
        auto & Socket = _Native.As<xNativeTcpSocket>();
        auto BufferPtr = _WritePacketBufferQueue.Peek();
        assert (BufferPtr);
        Socket.async_write_some(xAsioConstBuffer{BufferPtr->Buffer, BufferPtr->DataSize}, [this, BufferPtr](const xAsioError & Error, size_t TransferedBytes) {
            if (Error) {
                OnError();
                return;
            }
            assert(BufferPtr == _WritePacketBufferQueue.Peek());
            assert(TransferedBytes <= BufferPtr->DataSize);
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

    size_t xTcpConnection::PostData(const void * DataPtr_, size_t DataSize)
    {
        assert(DataSize);
        bool ExecWriteCall = (!_WriteDataSize && _Connected);
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
