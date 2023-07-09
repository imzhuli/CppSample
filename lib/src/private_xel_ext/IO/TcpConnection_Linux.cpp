#include <xel_ext/IO/TcpConnection.hpp>
#include <xel/String.hpp>

#if defined(X_SYSTEM_LINUX)

#include <cinttypes>
#include <fcntl.h>

// linux < 3.9
#ifndef SO_REUSEPORT
#define SO_REUSEPORT SO_REUSEADDR
#endif

// linux > 3.9 or bsd
#define X_ENABLE_REUSEPORT SO_REUSEPORT
#if defined(SO_REUSEPORT_LB)
#undef  X_ENABLE_REUSEPORT
#define X_ENABLE_REUSEPORT SO_REUSEPORT_LB
#endif

X_NS
{

    bool xTcpConnection::Init(xIoContext * IoContextPtr, xSocket && NativeHandle, iListener * ListenerPtr)
    {
        auto FailSafe = xScopeGuard{[=]{
            XelCloseSocket(NativeHandle);
            X_DEBUG_RESET(_IoContextPtr);
            X_DEBUG_RESET(_ListenerPtr);
        }};

        int flags = fcntl(NativeHandle, F_GETFL);
        fcntl(NativeHandle, F_SETFL, flags | O_NONBLOCK);
        setsockopt(NativeHandle, SOL_SOCKET, X_ENABLE_REUSEPORT, (char *)X2Ptr(int(1)), sizeof(int));

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
        _Status = eStatus::Connected;
        _SuspendReading = false;
        _HasPendingWriteFlag = false;
        SetAvailable();

        FailSafe.Dismiss();
        return true;
    }

    bool xTcpConnection::Init(xIoContext * IoContextPtr, const xNetAddress & Address, iListener * ListenerPtr)
    {
        int AF = AF_UNSPEC;
        sockaddr_storage AddrStorage = {};
        size_t AddrLen = Address.Dump(&AddrStorage);
        if (Address.IsV4()) {
            AF = AF_INET;
        } else if (Address.IsV6()) {
            AF = AF_INET6;
        }
        else {
            X_DEBUG_PRINTF("Invalid target address\n");
            return false;
        }

        assert(_Socket == InvalidSocket);
        _Socket = socket(AF, SOCK_STREAM, 0);
        if (_Socket == InvalidSocket) {
            X_DEBUG_PRINTF("Failed to create socket\n");
            return false;
        }

        auto FailSafe = xScopeGuard{[this]{
            XelCloseSocket(X_DEBUG_STEAL(_Socket, InvalidSocket));
            X_DEBUG_RESET(_IoContextPtr);
            X_DEBUG_RESET(_ListenerPtr);
        }};

        int flags = fcntl(_Socket, F_GETFL);
        fcntl(_Socket, F_SETFL, flags | O_NONBLOCK);
        setsockopt(_Socket, SOL_SOCKET, X_ENABLE_REUSEPORT, (char *)X2Ptr(int(1)), sizeof(int));

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
        _SuspendReading = false;
        _HasPendingWriteFlag = false;
        SetAvailable();

        FailSafe.Dismiss();
        return true;
    }

    void xTcpConnection::TrySendData()
    {
        while(auto WriteBufferPtr = _WriteBufferChain.Peek()) {
            ssize_t SendSize = send(_Socket, WriteBufferPtr->Buffer, WriteBufferPtr->DataSize, XelNoWriteSignal);
            if (SendSize == -1) {
                if (errno == EAGAIN) {
                    if (!Steal(_RequireOutputEvent, true)) {
                        UpdateEventTrigger();
                    }
                    return;
                }
                _IoContextPtr->PostError(*this);
                return;
            }
            if ((WriteBufferPtr->DataSize -= SendSize)) {
                memmove(WriteBufferPtr->Buffer, WriteBufferPtr->Buffer + SendSize, WriteBufferPtr->DataSize);
                if (!Steal(_RequireOutputEvent, true)) {
                    UpdateEventTrigger();
                }
                return;
            }
            _WriteBufferChain.RemoveFront();
            delete WriteBufferPtr;
        }
        _HasPendingWriteFlag = false;
        if (Steal(_RequireOutputEvent)) {
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
