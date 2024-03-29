#include <xel_ext/IO/TcpConnection.hpp>
#include <xel/String.hpp>
#include <cinttypes>

#if defined(X_SYSTEM_DARWIN)

#include <fcntl.h>

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
        setsockopt(NativeHandle, SOL_SOCKET, SO_NOSIGPIPE, (char *)X2Ptr(int(1)), sizeof(int));
        setsockopt(NativeHandle, SOL_SOCKET, X_ENABLE_REUSEPORT, (char *)X2Ptr(int(1)), sizeof(int));

        struct kevent Event[2] = {};
        Event[0].ident = NativeHandle;
        Event[0].flags = EV_ADD | EV_CLEAR;
        Event[0].filter = EVFILT_READ;
        Event[0].udata = this;
        Event[1].ident = NativeHandle;
        Event[1].flags = EV_ADD | EV_DISABLE | EV_CLEAR;
        Event[1].filter = EVFILT_WRITE;
        Event[1].udata = this;
        if (-1 == kevent(*IoContextPtr, Event, 2, nullptr, 0, nullptr)) {
            X_DEBUG_PRINTF("xTcpConnection::Init failed to register kevent\n");
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
        setsockopt(_Socket, SOL_SOCKET, SO_NOSIGPIPE, (char *)X2Ptr(int(1)), sizeof(int));
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

        struct kevent Event[2] = {};
        Event[0].ident = _Socket;
        Event[0].flags = EV_ADD | EV_CLEAR;
        Event[0].filter = EVFILT_READ;
        Event[0].udata = this;
        Event[1].ident = _Socket;
        Event[1].flags = EV_ADD | (_RequireOutputEvent ? 0 : EV_DISABLE) | EV_CLEAR;
        Event[1].filter = EVFILT_WRITE;
        Event[1].udata = this;
        if (-1 == kevent(*IoContextPtr, Event, 2, nullptr, 0, nullptr)) {
            X_DEBUG_PRINTF("xTcpConnection::Init failed to register kevent\n");
            return false;
        }

        _IoContextPtr = IoContextPtr;
        _ListenerPtr = ListenerPtr;
        _ReadBufferDataSize = 0;
        _SuspendReading = false;
        _HasPendingWriteFlag = false;

        FailSafe.Dismiss();
        SetAvailable();
        return true;
    }

    void xTcpConnection::TrySendData()
    {
        while(auto WriteBufferPtr = _WriteBufferChain.Peek()) {
            ssize_t SendSize = send(_Socket, WriteBufferPtr->Buffer, WriteBufferPtr->DataSize, XelNoWriteSignal);
            if (SendSize == -1) {
                if (errno == EAGAIN) {
                    if (!Steal(_RequireOutputEvent, true)) {
                        EnableWritingTrigger();
                    }
                    return;
                }
                _IoContextPtr->PostError(*this);
                return;
            }
            if ((WriteBufferPtr->DataSize -= SendSize)) {
                memmove(WriteBufferPtr->Buffer, WriteBufferPtr->Buffer + SendSize, WriteBufferPtr->DataSize);
                if (!Steal(_RequireOutputEvent, true)) {
                    EnableWritingTrigger();
                }
                return;
            }
            _WriteBufferChain.RemoveFront();
            delete WriteBufferPtr;
        }
        _HasPendingWriteFlag = false;
        if (Steal(_RequireOutputEvent)) {
            DisableWritingTrigger();
        }
        return;
    }

    void xTcpConnection::SuspendReading()
    {
        if (_SuspendReading) {
            return;
        }
        _SuspendReading = true;
        DisableReadingTrigger();
    }

    void xTcpConnection::ResumeReading()
    {
        if (!_SuspendReading) {
            return;
        }
        _SuspendReading = false;
        EnableReadingTrigger();
    }

    void xTcpConnection::EnableReadingTrigger()
    {
        struct kevent Event = {};
        Event.ident = _Socket;
        Event.flags = EV_ENABLE | EV_CLEAR;
        Event.filter = EVFILT_READ;
        Event.udata = this;
        if (-1 == kevent(*_IoContextPtr, &Event, 1, nullptr, 0, nullptr)) {
            X_DEBUG_PRINTF("kevent error: %s\n", strerror(errno));
            Fatal("EnableReadingTrigger: Failed to update kevent");
        }
    }

    void xTcpConnection::DisableReadingTrigger()
    {
        struct kevent Event = {};
        Event.ident = _Socket;
        Event.flags = EV_DISABLE | EV_CLEAR;
        Event.filter = EVFILT_READ;
        Event.udata = this;
        if (-1 == kevent(*_IoContextPtr, &Event, 1, nullptr, 0, nullptr)) {
            X_DEBUG_PRINTF("kevent error: %s\n", strerror(errno));
            Fatal("DisableReadingTrigger: Failed to update kevent");
        }
    }

    void xTcpConnection::EnableWritingTrigger()
    {
        struct kevent Event = {};
        Event.ident = _Socket;
        Event.flags = EV_ENABLE | EV_CLEAR;
        Event.filter = EVFILT_WRITE;
        Event.udata = this;
        if (-1 == kevent(*_IoContextPtr, &Event, 1, nullptr, 0, nullptr)) {
            X_DEBUG_PRINTF("kevent error: %s\n", strerror(errno));
            Fatal("EnableWritingTrigger: Failed to update kevent");
        }
    }

    void xTcpConnection::DisableWritingTrigger()
    {
        struct kevent Event = {};
        Event.ident = _Socket;
        Event.flags = EV_DISABLE | EV_CLEAR;
        Event.filter = EVFILT_WRITE;
        Event.udata = this;
        if (-1 == kevent(*_IoContextPtr, &Event, 1, nullptr, 0, nullptr)) {
            X_DEBUG_PRINTF("kevent error: %s\n", strerror(errno));
            Fatal("DisableWritingTrigger: Failed to update kevent");
        }
    }

}

#endif
