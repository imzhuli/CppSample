#include <xel_ext/IO/TcpConnection.hpp>
#include <xel/String.hpp>
#include <cinttypes>
#include <fcntl.h>

#if defined(X_SYSTEM_LINUX)

X_NS
{

    bool xTcpConnection::Init(xIoContext * IoContextPtr, xSocket NativeHandle, iListener * ListenerPtr)
    {
        assert(NativeHandle != InvalidSocket);
        X_DEBUG_PRINTF("xTcpConnection::Init NewConnection poller=%i socket=%i\n", (int)*IoContextPtr, NativeHandle);

        int flags = fcntl(NativeHandle, F_GETFL);
        fcntl(NativeHandle, F_SETFL, flags | O_NONBLOCK);

        struct epoll_event Event = {};
        Event.data.ptr = this;
        Event.events = EPOLLIN;
        if (-1 == epoll_ctl(*IoContextPtr, EPOLL_CTL_ADD, NativeHandle, &Event)) {
            X_DEBUG_PRINTF("xTcpConnection::Init failed to register epoll event\n");
            return false;
        }

        _Socket = NativeHandle;
        _IoContextPtr = IoContextPtr;
        _ListenerPtr = ListenerPtr;
        _ReadBufferDataSize = 0;
        _WriteBufferDataSize = 0;
        _WriteBufferPtr = nullptr;
        _Status = eStatus::Connected;
        return true;
    }

    bool xTcpConnection::Init(xIoContext * IoContextPtr, const xNetAddress & Address, iListener * ListenerPtr)
    {
        int AF = AF_UNSPEC;
        sockaddr_storage AddrStorage;
        size_t AddrLen = 0;
        memset(&AddrStorage, 0, sizeof(AddrStorage));
        if (Address.IsV4()) {
            auto & Addr4 = (sockaddr_in&)AddrStorage;
            Addr4.sin_family = AF_INET;
            Addr4.sin_addr = (decltype(sockaddr_in::sin_addr)&)(Address.Ipv4);
            Addr4.sin_port = htons(Address.Port);
            AddrLen = sizeof(sockaddr_in);
            AF = AF_INET;
        } else if (Address.IsV6()) {
            auto & Addr6 = (sockaddr_in6&)AddrStorage;
            Addr6.sin6_family = AF_INET6;
            Addr6.sin6_addr = (decltype(sockaddr_in6::sin6_addr)&)(Address.Ipv6);
            Addr6.sin6_port = htons(Address.Port);
            AddrLen = sizeof(sockaddr_in6);
            AF = AF_INET6;
        }
        else {
            return false;
        }

        assert(_Socket == InvalidSocket);
        _Socket = InvalidSocket;

        auto FailSafe = xScopeGuard{[this]{
            if (_Socket != InvalidSocket) {
                XelCloseSocket(X_DEBUG_STEAL(_Socket, InvalidSocket));
            }
            X_DEBUG_RESET(_IoContextPtr);
            X_DEBUG_RESET(_ListenerPtr);
        }};

        _Socket = socket(AF_INET, SOCK_STREAM, 0);
        if (-1 == Socket) {
            X_DEBUG_PRINTF("Failed to create socket\n");
            return false;
        }

        int flags = fcntl(NativeHandle, F_GETFL);
        fcntl(NativeHandle, F_SETFL, flags | O_NONBLOCK);
        return false;
    }

    void xTcpConnection::Clean()
    {
        XelCloseSocket(X_DEBUG_STEAL(_Socket, InvalidSocket));
    }

    void xTcpConnection::OnIoEventInReady()
    {
        size_t TotalSpace = sizeof(_ReadBuffer) - _ReadBufferDataSize;
        assert(TotalSpace);

        int ReadSize = read(_Socket, _ReadBuffer + _ReadBufferDataSize, TotalSpace);
        if (0 == ReadSize) {
            X_DEBUG_PRINTF("xTcpConnection::OnIoEventInReady EOF\n");
            _Status = eStatus::Closed;
            _ListenerPtr->OnPeerClose(this);
            return;
        }
        if (-1 == ReadSize) {
            if (EAGAIN == ReadSize) {
                return;
            }
            SetUnavailable();
            return;
        }
        _ReadBufferDataSize += ReadSize;
        _ReadBufferDataSize -= _ListenerPtr->OnData(this, _ReadBuffer, _ReadBufferDataSize);
        return;
    }

    void xTcpConnection::OnIoEventOutReady()
    {
        TrySendData();
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

    void xTcpConnection::TrySendData()
    {
        if (!_WriteBufferPtr) {
            _WriteBufferPtr = _WriteBufferChain.Pop();
        }
        while(_WriteBufferPtr) {
            ssize_t SendSize = write(_Socket, _WriteBufferPtr->Buffer, _WriteBufferPtr->DataSize);
            if (SendSize == -1) {
                if (errno == EAGAIN) {
                    Fatal("Not implemented");
                    return;
                }
                SetUnavailable();
                return;
            }
            size_t RemainSize = _WriteBufferPtr->DataSize - SendSize;
            if (RemainSize) {
                memmove(_WriteBufferPtr->Buffer, _WriteBufferPtr->Buffer + SendSize, RemainSize);
                Fatal("Check if output event is required");
                return;
            }
            delete _WriteBufferPtr;
            _WriteBufferPtr = _WriteBufferChain.Pop();
        }
        return;
    }

}

#endif
