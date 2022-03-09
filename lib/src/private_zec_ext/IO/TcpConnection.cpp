
#include "./_Local.hpp"
#include "./TcpConnection.hpp"
#include <cstring>
#include <iostream>

ZEC_NS
{

    xTcpSocketContext::xTcpSocketContext(xIoContext * IoContextPtr, const xNetAddress & Address, uint64_t Port)
    : _Socket(xIoCaster()(*IoContextPtr))
    {
        _ReadBuffer[MaxPacketSize] = '\0';
        _ConnectionState = eConnecting;
        _Socket.async_connect(MakeTcpEndpoint(Address), [this, R=Retainer{*this}] (const xAsioError & Error) mutable {
            if (Error) {
                OnError();
                return;
            }
            OnConnected();
        });
    }

    xTcpSocketContext::xTcpSocketContext(xIoHandle Handle)
    : _Socket(std::move(Handle.AsRef<xTcpSocket>()))
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
        if (_ConnectionState >= eConnectionClosing) {
            return 0;
        }
        bool ExecWriteCall = (!_WriteDataSize && _ConnectionState == eConnected);
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

    void xTcpSocketContext::DoRead()
    {
        if (_ReadState != eReading) {
            return;
        }
        size_t BufferSize = MaxPacketSize - _ReadDataSize;
        _Socket.async_read_some(xAsioMutableBuffer{_ReadBuffer + _ReadDataSize, BufferSize}, [this, R=Retainer{*this}](const xAsioError & Error, size_t TransferedSize) {
            if (Error) {
                if (Error == asio::error::eof) {
                    _ConnectionState = eConnectionClosed;
                    if (auto ListenerPtr = Steal(_ListenerPtr)) {
                        ListenerPtr->OnPeerClose(this);
                        DoClose();
                    }
                    return;
                }
                OnError();
                return;
            }
            auto EntryGuard = _ReadCallbackEntry.Guard();
            _ReadDataSize += TransferedSize;
            auto DataPtr = _ReadBuffer;
            auto DataSize = _ReadDataSize;
            while(DataSize) {
                if (!_ListenerPtr) {
                    _ReadState = eReadSuspended;
                    _ReadDataSize = 0;
                    return;
                }
                size_t ConsumedDataSize = _ListenerPtr->OnData(this, DataPtr, DataSize);
                DataPtr += ConsumedDataSize;
                DataSize -= ConsumedDataSize;
                if (!ConsumedDataSize || _ReadState != eReading) {
                    assert(DataSize);
                    memmove(_ReadBuffer, DataPtr, DataSize);
                    break;
                }
            }
            _ReadDataSize = DataSize;
            DoRead();
        });
    }

    void xTcpSocketContext::SuspendReading()
    {
        _ReadState = eReadSuspended;
    }

    void xTcpSocketContext::ResumeReading()
    {
        _ReadState = eReading;
        auto Guard = _ReadCallbackEntry.Guard();
        if (!Guard) {
            DoRead();
        }
    }

    void xTcpSocketContext::DoFlush()
    {
        auto BufferPtr = _WritePacketBufferQueue.Peek();
        if (!BufferPtr) {
            if (_ConnectionState == eConnectionClosing) {
                DoClose();
            }
            return;
        }

        _Socket.async_write_some(xAsioConstBuffer{BufferPtr->Buffer, BufferPtr->DataSize}, [this, R=Retainer{*this}, BufferPtr](const xAsioError & Error, size_t TransferedBytes) {
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

    void xTcpSocketContext::Close()
    {
        _ListenerPtr = nullptr;
        DoClose();
    }

    void xTcpSocketContext::DoClose()
    {
        _Socket.close(X2Ref(xAsioError{}));
    }

    void xTcpSocketContext::OnConnected()
    {
        _ConnectionState = eConnected;
        if (_ListenerPtr) {
            _ListenerPtr->OnConnected(this);
            DoRead();
            if (!_WritePacketBufferQueue.IsEmpty()) {
                DoFlush();
            }
        }
    }

    void xTcpSocketContext::OnError()
    {
        if (_ConnectionState >= eConnectionClosing) {
            return;
        }
        _ConnectionState = eConnectionError;
        if (auto ListenerPtr = Steal(_ListenerPtr)) {
            ListenerPtr->OnError(this);
        }
        return;
    }

//     /* xTcpConnection */

//     xTcpConnection::xTcpConnection()
//     {
//         _NativeContext.CreateAs<xSharedTcpSocketContextPtr>();
//     }

//     xTcpConnection::~xTcpConnection()
//     {
//         auto & Context = _NativeContext.As<xSharedTcpSocketContextPtr>();
//         assert(!Context);
//         _NativeContext.DestroyAs<xSharedTcpSocketContextPtr>();
//     }

//     bool xTcpConnection::Init(xIoContext * IoContextPtr, xIoHandle NativeHandle, iListener * ListenerPtr)
//     {
//         _IoContextPtr = IoContextPtr;
//         auto & Context = _NativeContext.As<xSharedTcpSocketContextPtr>();
//         Context.reset(new xTcpSocketContext(NativeHandle));
//         _ListenerPtr = ListenerPtr;
//         return true;
//     }

//     bool xTcpConnection::Init(xIoContext * IoContextPtr, const char * Ip, uint64_t Port, iListener * ListenerPtr)
//     {
//         xNetAddress Address = xNetAddress::Make(Ip);
//         return Init(IoContextPtr, Address, Port, ListenerPtr);
//     }

//     bool xTcpConnection::Init(xIoContext * IoContextPtr, const xNetAddress & Address, uint64_t Port, iListener * ListenerPtr)
//     {
//         assert(IoContextPtr);
//         assert(ListenerPtr);
//         assert(Address);
//         assert(Port);

//         _IoContextPtr = IoContextPtr;
//         auto & Context = _NativeContext.As<xSharedTcpSocketContextPtr>();
//         Context.reset(new xTcpSocketContext(IoContextPtr));
//         _ListenerPtr = ListenerPtr;
//         return true;
//     }

//     void xTcpConnection::Clean()
//     {
//         _Connected = false;
//         _ListenerPtr = nullptr;

//         auto & Context = _NativeContext.As<xSharedTcpSocketContextPtr>();
//         if(Context) {
//             _IoContextPtr->SetFinalExpiration(*Context);
//             Context.reset();
//         }
//     }

//     size_t xTcpConnection::PostData(const void * DataPtr, size_t DataSize)
//     {
//         auto & Context = _NativeContext.As<xSharedTcpSocketContextPtr>();
//         return Context->PostData(DataPtr, DataSize);
//     }

}
