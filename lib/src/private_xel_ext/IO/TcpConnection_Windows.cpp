#include <xel_ext/IO/TcpConnection.hpp>
#include <cinttypes>

#if defined(X_SYSTEM_WINDOWS)

X_NS
{

    bool xTcpConnection::Init(xIoContext * IoContextPtr, xSocket NativeHandle, iListener * ListenerPtr)
    {
        assert(NativeHandle != InvalidSocket);
        assert(!_ListenerPtr && ListenerPtr);
        X_DEBUG_PRINTF("xTcpConnection::Init IoContextPtr=%p, Socket=%" PRIuPTR "\n", IoContextPtr, (uintptr_t&)NativeHandle);

        if (CreateIoCompletionPort((HANDLE)NativeHandle, *IoContextPtr, (ULONG_PTR)this, 0) == NULL) {
            X_DEBUG_PRINTF("xTcpConnection::Init failed to create competion port\n");
            return false;
        }

        memset(&_ReadOverlappedObject, 0, sizeof(_ReadOverlappedObject));
        memset(&_WriteOverlappedObject, 0, sizeof(_WriteOverlappedObject));        
        
        _SendingBufferPtr = nullptr;
        _Socket = NativeHandle;
        _IoContextPtr = IoContextPtr;
        _ListenerPtr = ListenerPtr;
        _Status = eStatus::Connected;

        TryRecvData();
        if (!IsAvailable()) {
            return false;
        }
        return true;
    }

    void xTcpConnection::Clean()
    {
        XelCloseSocket(X_DEBUG_STEAL(_Socket, InvalidSocket));
        X_DEBUG_RESET(_ListenerPtr);
        X_DEBUG_RESET(_Status);
    }

    void xTcpConnection::OnIoEventInReady()
    {
        if (!_ReadDataSize) {
            _Status = eStatus::Closed;
            _ListenerPtr->OnPeerClose(this);
            return;
        }
        size_t TotalSize = (_ReadBufferUsage.buf - (CHAR*)_ReadBuffer) + _ReadDataSize;
        size_t SkipSize  = TotalSize - _ListenerPtr->OnData(this, _ReadBuffer, TotalSize);
        TryRecvData(SkipSize);
    }

    void xTcpConnection::OnIoEventOutReady() 
    {
        X_DEBUG_PRINTF("xTcpConnection::OnIoEventOutputReady Instance=%p, TransferedData=%zi\n", this, (size_t)_SentDataSize);
    }

    size_t xTcpConnection::PostData(const void * DataPtr_, size_t DataSize)
    {
        assert(DataPtr_ && DataSize);
        assert(_Status != eStatus::Unspecified);
        
        if (_Status >= eStatus::Closing) {
            return 0;
        }

        auto DataPtr = (const ubyte*)DataPtr_;
        auto Packets = DataSize / sizeof(xPacketBuffer::Buffer);
        for (size_t i = 0 ; i < Packets; ++i) {
            auto BufferPtr = new xPacketBuffer;
            memcpy(BufferPtr->Buffer, DataPtr, sizeof(xPacketBuffer::Buffer));
            BufferPtr->DataSize = sizeof(xPacketBuffer::Buffer);
            DataPtr  += sizeof(xPacketBuffer::Buffer);
            DataSize -= sizeof(xPacketBuffer::Buffer);
            _WriteBufferChain.Push(BufferPtr);
        }
        if (DataSize) {
            auto BufferPtr = new xPacketBuffer;
            memcpy(BufferPtr->Buffer, DataPtr, DataSize);
            BufferPtr->DataSize = DataSize;
            _WriteBufferChain.Push(BufferPtr);
        }
        
        if (_Status == eStatus::Connecting) {
            return DataSize;
        }

        // _Status == eStatus::Connected
        TrySendData();
        return DataSize;
    }

    void xTcpConnection::TryRecvData(size_t SkipSize)
    {
        _ReadBufferUsage.buf = (CHAR*)_ReadBuffer + SkipSize;
        _ReadBufferUsage.len = (ULONG)(sizeof(_ReadBuffer) - SkipSize);
        _ReadDataSize = 0;
        _ReadFlags = 0;
        assert(_ReadBufferUsage.len);
        auto Result = WSARecv(_Socket, &_ReadBufferUsage, 1, &_ReadDataSize, &_ReadFlags, &_ReadOverlappedObject, nullptr);
        if (Result && WSAGetLastError() != WSA_IO_PENDING) {
            SetUnavailable();
        }
    }

    void xTcpConnection::TrySendData()
    {
        assert(!_SendingBufferPtr);
        _SendingBufferPtr = _WriteBufferChain.Pop();
        if (!_SendingBufferPtr) {
            return;
        }
        _WriteBufferUsage.buf = (CHAR*)_SendingBufferPtr->Buffer;
        _WriteBufferUsage.len = (ULONG)_SendingBufferPtr->DataSize;
        _SentDataSize = 0;
        auto Result = WSASend(_Socket, &_WriteBufferUsage, 1, &_SentDataSize, 0, &_WriteOverlappedObject, nullptr);
        if (Result && WSAGetLastError() != WSA_IO_PENDING) {
            // Pending Error Procedure
            SetUnavailable();
            return;
        }
        return;
    }

}

#endif
