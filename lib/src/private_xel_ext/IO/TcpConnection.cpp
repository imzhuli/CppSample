#include <xel_ext/IO/TcpConnection.hpp>
#include <climits>

X_NS
{

    void xTcpConnection::Clean()
    {
        if (_WriteBufferPtr) {
            delete _WriteBufferPtr;
            while(auto WriteBufferPtr = _WriteBufferChain.Pop()) {
                delete WriteBufferPtr;
            }
        }
        XelCloseSocket(X_DEBUG_STEAL(_Socket, InvalidSocket));
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
