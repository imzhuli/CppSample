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
        Event.events = EPOLLET | EPOLLIN;
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
        _SuspendReading = false;
        return true;
    }

    bool xTcpConnection::Init(xIoContext * IoContextPtr, const xNetAddress & Address, iListener * ListenerPtr)
    {
        int AF = AF_UNSPEC;
        sockaddr_storage AddrStorage = {};
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
            X_DEBUG_PRINTF("Invalid target address\n");
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

        _Socket = socket(AF, SOCK_STREAM, 0);
        if (_Socket == InvalidSocket) {
            X_DEBUG_PRINTF("Failed to create socket\n");
            return false;
        }

        int flags = fcntl(_Socket, F_GETFL);
        fcntl(_Socket, F_SETFL, flags | O_NONBLOCK);

        auto ConnectResult = connect(_Socket, (sockaddr*)&AddrStorage, AddrLen);
        if (ConnectResult == -1) {
            auto Error = errno;
            if (Error != EINPROGRESS) {
                X_DEBUG_PRINTF("Failed to start connection, Socket=%i Error=%s\n", _Socket,  strerror(Error));
                return false;
            }
            _Status = eStatus::Connecting;
            _RequireOutputEvent = true;
        }
        else {
            _Status = eStatus::Connected;
            _RequireOutputEvent = false;
        }

        struct epoll_event Event = {};
        Event.data.ptr = this;
        Event.events = EPOLLET | EPOLLIN | (_RequireOutputEvent ? EPOLLOUT : 0);
        if (-1 == epoll_ctl(*IoContextPtr, EPOLL_CTL_ADD, _Socket, &Event)) {
            X_DEBUG_PRINTF("xTcpConnection::Init failed to register epoll event\n");
            return false;
        }
        _IoContextPtr = IoContextPtr;
        _ListenerPtr = ListenerPtr;
        _ReadBufferDataSize = 0;
        _WriteBufferDataSize = 0;
        _WriteBufferPtr = nullptr;
        _SuspendReading = false;

        FailSafe.Dismiss();
        return true;
    }

    void xTcpConnection::Clean()
    {
        XelCloseSocket(X_DEBUG_STEAL(_Socket, InvalidSocket));
    }

    void xTcpConnection::OnIoEventInReady()
    {
        X_DEBUG_PRINTF("xTcpConnection::OnIoEventInReady\n");
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
        X_DEBUG_PRINTF("xTcpConnection::OnIoEventOutReady\n");
        if (_Status == eStatus::Connecting) {
            X_DEBUG_PRINTF("Connection established\n");
            _Status = eStatus::Connected;
            _ListenerPtr->OnConnected(this);
        }
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
            ssize_t SendSize = send(_Socket, _WriteBufferPtr->Buffer, _WriteBufferPtr->DataSize, XelNoWriteSignal);
            if (SendSize == -1) {
                if (errno == EAGAIN) {
                    if (!_RequireOutputEvent) {
                        _RequireOutputEvent = true;
                        UpdateEventTrigger();
                    }
                    return;
                }
                SetUnavailable();
                return;
            }
            size_t RemainSize = _WriteBufferPtr->DataSize - SendSize;
            if (RemainSize) {
                memmove(_WriteBufferPtr->Buffer, _WriteBufferPtr->Buffer + SendSize, RemainSize);
                if (!_RequireOutputEvent) {
                    _RequireOutputEvent = true;
                    UpdateEventTrigger();
                }
                return;
            }
            delete _WriteBufferPtr;
            _WriteBufferPtr = _WriteBufferChain.Pop();
        }
        if (_RequireOutputEvent) {
            _RequireOutputEvent = false;
            UpdateEventTrigger();
        }
        return;
    }

    void xTcpConnection::SuspendReading()
    {
        if (_SuspendReading) {
            return;
        }
        _SuspendReading = true;
        UpdateEventTrigger();
    }

    void xTcpConnection::ResumeReading()
    {
        if (!_SuspendReading) {
            return;
        }
        _SuspendReading = false;
        UpdateEventTrigger();
    }

    void xTcpConnection::UpdateEventTrigger()
    {
        struct epoll_event Event = {};
        Event.data.ptr = this;
        Event.events = EPOLLET | (_SuspendReading ? 0 : EPOLLIN) | (_RequireOutputEvent ? EPOLLOUT : 0);
        if (-1 == epoll_ctl(*_IoContextPtr, EPOLL_CTL_MOD, _Socket, &Event)) {
            X_DEBUG_PRINTF("xTcpConnection::UpdateEventTrigger failed to modify epoll event, Reason=%s\n", strerror(errno));
            Fatal("");
            return;
        }
    }

}

#endif
