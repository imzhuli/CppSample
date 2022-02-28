#include <zec_ext/IO/TcpConnection.hpp>
#include <cstring>
#include "./_Local.hpp"

#include <iostream>

ZEC_NS
{

    xTcpSocketContext::xTcpSocketContext(xIoContext * IoContextPtr, const xNetAddress & Address, uint64_t Port)
    : Socket(*IOUtil::Native(IoContextPtr))
    {
        _ReadBuffer[MaxPacketSize] = '\0';
        Socket.async_connect(MakeEndpoint(Address, Port), [this](const xAsioError & Error) {
            if (Error) {
                OnError();
                return;
            }
            OnConnected();
        });
    }

    xTcpSocketContext::xTcpSocketContext(xIoNativeHandle NativeHandle)
    : Socket(std::move(NativeHandle.GetRefAs<xNativeTcpSocket>()))
    {
        _ReadBuffer[MaxPacketSize] = '\0';
    }

    xTcpSocketContext::~xTcpSocketContext()
    {
        while(auto WriteBufferPtr = _WritePacketBufferQueue.Pop()) {
            DeleteWriteBuffer(WriteBufferPtr);
        }
    }

    size_t xTcpSocketContext::PostData(const void * DataPtr_, size_t DataSize)
    {
        assert(DataSize);
        if (_Error) {
            return 0;
        }
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

    void xTcpSocketContext::DoFlush()
    {
        auto BufferPtr = _WritePacketBufferQueue.Peek();
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
                return;
            }
        });
    }

    void xTcpSocketContext::OnConnected()
    {
        _Connected = true;
        DoRead();
        if (!_WritePacketBufferQueue.IsEmpty()) {
            DoFlush();
        }
    }

    void xTcpSocketContext::OnError()
    {
        if (_Error) {
            return;
        }
        _Error = true;
        _Connected = false;
        return;
    }

    /* xTcpConnection */

    xTcpConnection::xTcpConnection()
    {
        _NativeContext.CreateAs<xSharedTcpSocketContextPtr>();
    }

    xTcpConnection::~xTcpConnection()
    {
        auto & Context = _NativeContext.As<xSharedTcpSocketContextPtr>();
        assert(!Context);
        _NativeContext.DestroyAs<xSharedTcpSocketContextPtr>();
    }

    bool xTcpConnection::Init(xIoContext * IoContextPtr, xIoNativeHandle NativeHandle, iListener * ListenerPtr)
    {
        _IoContextPtr = IoContextPtr;
        auto & Context = _NativeContext.As<xSharedTcpSocketContextPtr>();
        Context.reset(new xTcpSocketContext(NativeHandle));
        _ListenerPtr = ListenerPtr;
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

        _IoContextPtr = IoContextPtr;
        auto & Context = _NativeContext.As<xSharedTcpSocketContextPtr>();
        Context.reset(new xTcpSocketContext(IoContextPtr));
        _ListenerPtr = ListenerPtr;
        return true;
    }

    void xTcpConnection::Clean()
    {
        _Connected = false;
        _ListenerPtr = nullptr;

        auto & Context = _NativeContext.As<xSharedTcpSocketContextPtr>();
        if(Context) {
            _IoContextPtr->SetFinalExpiration(*Context);
            Context.reset();
        }
    }

    size_t xTcpConnection::PostData(const void * DataPtr, size_t DataSize)
    {
        auto & Context = _NativeContext.As<xSharedTcpSocketContextPtr>();
        return Context->PostData(DataPtr, DataSize);
    }

}
