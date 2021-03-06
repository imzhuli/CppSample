
#include "./TcpConnection.hpp"
#include "./NetBase.hpp"
#include <cstring>

ZEC_NS
{

    xTcpSocketContext::xTcpSocketContext(xIoContext * IoContextPtr, const xNetAddress & Address, xTcpConnection * TcpConnectionPtr, xTcpConnection::iListener * ListenerPtr)
    : _Socket(xIoCaster()(*IoContextPtr))
    {
        _ReadBuffer[MaxPacketSize] = '\0';
        _ConnectionState = eConnecting;
        _ListenerContextPtr = TcpConnectionPtr;
        _ListenerPtr = ListenerPtr;
        _Socket.async_connect(MakeTcpEndpoint(Address), [this, R=xRetainer{this}] (const xAsioError & Error) {
            if (Error) {
                OnError();
                return;
            }
            OnConnected();
        });
    }

    xTcpSocketContext::xTcpSocketContext(xIoHandle Handle, xTcpConnection * TcpConnectionPtr, xTcpConnection::iListener * ListenerPtr)
    : _Socket(std::move(Handle.AsRef<xTcpSocket>()))
    {
        _ReadBuffer[MaxPacketSize] = '\0';
        _ConnectionState = eConnected;
        _ListenerContextPtr = TcpConnectionPtr;
        _ListenerPtr = ListenerPtr;
        ResumeReading();
    }

    xTcpSocketContext::~xTcpSocketContext()
    {
        assert(_ConnectionState == eConnectionClosed);
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

    void xTcpSocketContext::DoReadCallback()
    {
        auto DataPtr = _ReadBuffer;
        auto DataSize = _ReadDataSize;
        while(DataSize) {
            if (!_ListenerPtr) {
                _ReadState = eReadSuspended;
                _ReadDataSize = 0;
                return;
            }
            size_t ConsumedDataSize = _ListenerPtr->OnData(_ListenerContextPtr, DataPtr, DataSize);
            DataPtr += ConsumedDataSize;
            DataSize -= ConsumedDataSize;
            if (!ConsumedDataSize || _ReadState != eReading) {
                if(DataSize) {
                    memmove(_ReadBuffer, DataPtr, DataSize);
                }
                break;
            }
        }
        _ReadDataSize = DataSize;
    }

    void xTcpSocketContext::DoRead()
    {
        if (_ReadState != eReading) {
            return;
        }
        size_t BufferSize = MaxPacketSize - _ReadDataSize;
        _Socket.async_read_some(xAsioMutableBuffer{_ReadBuffer + _ReadDataSize, BufferSize}, [this, R=xRetainer{this}, EntryGuard=_ReadCallbackEntry.Guard()](const xAsioError & Error, size_t TransferedSize) {
            if (Error) {
                if (Error == asio::error::eof) {
                    _ConnectionState = eConnectionClosed;
                    if (auto ListenerPtr = Steal(_ListenerPtr)) {
                        ListenerPtr->OnPeerClose(Steal(_ListenerContextPtr));
                        DoClose();
                    }
                    return;
                }
                OnError();
                return;
            }
            _TotalReadSize += TransferedSize;
            _ReadDataSize += TransferedSize;
            DoReadCallback();
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
        if (auto Guard = _ReadCallbackEntry.Guard()) {
            DoReadCallback();
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

        _Socket.async_write_some(xAsioConstBuffer{BufferPtr->Buffer, BufferPtr->DataSize}, [this, R=xRetainer{this}, BufferPtr](const xAsioError & Error, size_t TransferedBytes) {
            if (Error) {
                OnError();
                return;
            }
            _TotalWriteSize += TransferedBytes;

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

    bool xTcpSocketContext::GracefulClose()
    {
        if (_ConnectionState >= eConnectionClosing) {
            return false;
        }
        _ListenerPtr = nullptr;
        _ConnectionState = eConnectionClosing;
        _ReadState = eReadSuspended;
        if (!_WritePacketBufferQueue.Peek()) {
            DoClose();
            return true;
        }
        return false;
    }

    void xTcpSocketContext::Close()
    {
        _ListenerPtr = nullptr;
        DoClose();
    }

    void xTcpSocketContext::DoClose()
    {
        if (_ConnectionState == eConnectionClosed) {
            return;
        }
        _Socket.close(X2Ref(xAsioError{}));
        _ConnectionState = eConnectionClosed;
    }

    void xTcpSocketContext::OnConnected()
    {
        if (_ListenerPtr) {
            _ListenerPtr->OnConnected(_ListenerContextPtr);
            if (_ConnectionState != eConnecting) { // check if Listener callback closed connection
                return;
            }
            _ConnectionState = eConnected;
            if (auto Guard = _ReadCallbackEntry.Guard()) {
                DoRead();
            }
            if (!_WritePacketBufferQueue.IsEmpty()) {
                DoFlush();
            }
            return;
        }
        _ConnectionState = eConnected;
    }

    void xTcpSocketContext::OnError()
    {
        if (_ConnectionState >= eConnectionClosing) {
            return;
        }
        _ConnectionState = eConnectionError;
        if (auto ListenerPtr = Steal(_ListenerPtr)) {
            ListenerPtr->OnError(Steal(_ListenerContextPtr));
        }
        return;
    }

    /* xTcpConnection */

    xTcpConnection::xTcpConnection()
    {}

    xTcpConnection::~xTcpConnection()
    {
        assert(!_SocketPtr);
    }

    bool xTcpConnection::Init(xIoHandle NativeHandle, iListener * ListenerPtr)
    {
        assert(!_SocketPtr);
        try {
            _SocketPtr = new xTcpSocketContext(NativeHandle, this, ListenerPtr);
        } catch (...) {
            return false;
        }
        return true;
    }

    bool xTcpConnection::Init(xIoContext * IoContextPtr, const char * Ip, uint64_t Port, iListener * ListenerPtr)
    {
        xNetAddress Address = xNetAddress::Make(Ip, Port);
        return Init(IoContextPtr, Address, ListenerPtr);
    }

    bool xTcpConnection::Init(xIoContext * IoContextPtr, const xNetAddress & Address, iListener * ListenerPtr)
    {
        assert(!_SocketPtr);
        try {
            _SocketPtr = new xTcpSocketContext(IoContextPtr, Address, this, ListenerPtr);
        } catch (...) {
            return false;
        }
        return true;
    }

    bool xTcpConnection::GracefulClose()
    {
        assert(_SocketPtr);
        return _SocketPtr->GracefulClose();
    }

    void xTcpConnection::Clean()
    {
        auto SocketPtr = xRetainer{ NoRetain, Steal(_SocketPtr) };
        SocketPtr->Close();
    }

    xNetAddress xTcpConnection::GetRemoteAddress() const
    {
        assert(_SocketPtr);
        return MakeNetAddress(_SocketPtr->GetRemoteAddress());
    }

    xNetAddress xTcpConnection::GetLocalAddress() const
    {
        assert(_SocketPtr);
        return MakeNetAddress(_SocketPtr->GetLocalAddress());
    }

    void xTcpConnection::ResizeSendBuffer(size_t Size)
    {
        _SocketPtr->ResizeSendBuffer(Size);
    }

    void xTcpConnection::ResizeReceiveBuffer(size_t Size)
    {
        _SocketPtr->ResizeReceiveBuffer(Size);
    }

    size_t xTcpConnection::PostData(const void * DataPtr, size_t DataSize)
    {
        assert(_SocketPtr);
        return _SocketPtr->PostData(DataPtr, DataSize);
    }

    size_t xTcpConnection::GetPendingWriteBlockCount() const
    {
        return _SocketPtr ? _SocketPtr->GetPendingWriteBlocks() : 0;
    }

    void xTcpConnection::SuspendReading()
    {
        assert(_SocketPtr);
        _SocketPtr->SuspendReading();
    }

    void xTcpConnection::ResumeReading()
    {
        assert(_SocketPtr);
        _SocketPtr->ResumeReading();
    }

    xTcpConnection::xAudit xTcpConnection::GetAudit()
    {
        if (_SocketPtr) {
            return { _SocketPtr->GetTotalReadSize(), _SocketPtr->GetTotalWriteSize() };
        }
        return {0, 0};
    }

    xTcpConnection::xAudit xTcpConnection::StealAudit()
    {
        if (_SocketPtr) {
            return { _SocketPtr->StealTotalReadSize(), _SocketPtr->StealTotalWriteSize() };
        }
        return {0, 0};
    }

}
