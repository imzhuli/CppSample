#include <xel_ext/IO/TcpConnection.hpp>
#include <climits>

X_NS
{

#ifndef X_SYSTEM_WINDOWS

    void xTcpConnection::Clean()
    {
        while(auto WriteBufferPtr = _WriteBufferChain.Pop()) {
            delete WriteBufferPtr;
        }
        assert(_WriteBufferChain.IsEmpty());
        XelCloseSocket(X_DEBUG_STEAL(_Socket, InvalidSocket));
    }

    void xTcpConnection::OnIoEventInReady()
    {
        // X_DEBUG_PRINTF("xTcpConnection::OnIoEventInReady\n");

        while(true) {
            size_t TotalSpace = sizeof(_ReadBuffer) - _ReadBufferDataSize;
            assert(TotalSpace && "ReadBuffer size is larger than MaxPacketSize, listener should guarantee read buffer never be full");

            int ReadSize = read(_Socket, _ReadBuffer + _ReadBufferDataSize, TotalSpace);
            if (0 == ReadSize) {
                _Status = eStatus::Closing;
                SetDisabled();
                _ListenerPtr->OnPeerClose(this);
                return;
            }
            if (-1 == ReadSize) {
                auto Error = errno;
                if (EAGAIN == Error) {
                    return;
                }
                SetError();
                return;
            }
            _ReadBufferDataSize += ReadSize;
            auto ProcessDataPtr = (ubyte*)_ReadBuffer;
            while(_ReadBufferDataSize) {
                auto ProcessedData = _ListenerPtr->OnData(this, ProcessDataPtr, _ReadBufferDataSize);
                if (ProcessedData == InvalidPacketSize) {
                    SetError();
                    return;
                }
                if (!ProcessedData){
                    if (ProcessDataPtr != _ReadBuffer) { // some data are processed
                        memmove(_ReadBuffer, ProcessDataPtr, _ReadBufferDataSize);
                    }
                    break;
                }
                ProcessDataPtr += ProcessedData;
                _ReadBufferDataSize -= ProcessedData;
            }
        }
        return;
    }

    void xTcpConnection::OnIoEventOutReady()
    {
        if (_Status == eStatus::Connecting) {
            // X_DEBUG_PRINTF("Connection established\n");
            _Status = eStatus::Connected;

            auto NeedFlushEvent = _HasPendingWriteFlag;
            _ListenerPtr->OnConnected(this); // if PostData is called during callback, a TrySendData is called internally
            TrySendData();
            if (NeedFlushEvent && !_HasPendingWriteFlag) {
                _ListenerPtr->OnFlush(this);
            }
        }
        else {
            TrySendData();
            if (!_HasPendingWriteFlag) {
                _ListenerPtr->OnFlush(this);
            }
        }
    }

    size_t xTcpConnection::PostData(const void * DataPtr_, size_t DataSize)
    {
        assert(DataPtr_);
        assert(_Status != eStatus::Unspecified);

        if (!IsAvailable() || !DataSize) {
            return 0;
        }

        auto DataPtr = (const ubyte*)DataPtr_;
        auto RemainedSize = DataSize;
        auto Packets = RemainedSize / sizeof(xPacketBuffer::Buffer);
        for (size_t i = 0 ; i < Packets; ++i) {
            auto BufferPtr = new (std::nothrow) xPacketBuffer;
            if (!BufferPtr) {
                return DataSize - RemainedSize;
            }
            memcpy(BufferPtr->Buffer, DataPtr, sizeof(xPacketBuffer::Buffer));
            BufferPtr->DataSize = sizeof(xPacketBuffer::Buffer);
            DataPtr  += sizeof(xPacketBuffer::Buffer);
            RemainedSize -= sizeof(xPacketBuffer::Buffer);
            _WriteBufferChain.Push(BufferPtr);
        }
        if (RemainedSize) {
            auto BufferPtr = new (std::nothrow) xPacketBuffer;
            if (!BufferPtr) {
                return DataSize - RemainedSize;
            }
            memcpy(BufferPtr->Buffer, DataPtr, RemainedSize);
            BufferPtr->DataSize = RemainedSize;
            _WriteBufferChain.Push(BufferPtr);
        }

        _HasPendingWriteFlag = true;
        if (_Status == eStatus::Connecting) {
            return DataSize;
        }

        TrySendData();
        return DataSize;
    }
#endif

    xNetAddress xTcpConnection::GetRemoteAddress() const
    {
		sockaddr_storage SockAddr;
		socklen_t SockAddrLen = sizeof(SockAddr);
        if(getpeername(_Socket, (sockaddr*)&SockAddr, &SockAddrLen)) {
            return {};
        }
        return xNetAddress::Parse(&SockAddr);
    }

    xNetAddress xTcpConnection::GetLocalAddress() const
    {
		sockaddr_storage SockAddr;
		socklen_t SockAddrLen = sizeof(SockAddr);
        if(getpeername(_Socket, (sockaddr*)&SockAddr, &SockAddrLen)) {
            return {};
        }
        return xNetAddress::Parse(&SockAddr);
    }

    void xTcpConnection::ResizeSendBuffer(size_t Size)
    {
        assert(Size < INT_MAX);
        if (_Socket == InvalidSocket) {
            return;
        }
        setsockopt(_Socket, SOL_SOCKET, SO_SNDBUF, (char *)X2Ptr(int(Size)), sizeof(int));
    }

    void xTcpConnection::ResizeReceiveBuffer(size_t Size)
    {
        assert(Size < INT_MAX);
        if (_Socket == InvalidSocket) {
            return;
        }
        setsockopt(_Socket, SOL_SOCKET, SO_RCVBUF, (char *)X2Ptr(int(Size)), sizeof(int));
    }

}
