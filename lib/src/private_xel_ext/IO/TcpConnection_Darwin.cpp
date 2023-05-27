#include <xel_ext/IO/TcpConnection.hpp>
#include <xel/String.hpp>
#include <cinttypes>
#include <fcntl.h>

#if defined(X_SYSTEM_DARWIN)

X_NS
{

    bool xTcpConnection::Init(xIoContext * IoContextPtr, xSocket NativeHandle, iListener * ListenerPtr)
    {
        // X_DEBUG_PRINTF("xTcpConnection::Init NewConnection poller=%i socket=%i\n", (int)*IoContextPtr, NativeHandle);

        auto FailSafe = xScopeGuard{[=]{
            XelCloseSocket(NativeHandle);
            X_DEBUG_RESET(_IoContextPtr);
            X_DEBUG_RESET(_ListenerPtr);
        }};

        int flags = fcntl(NativeHandle, F_GETFL);
        fcntl(NativeHandle, F_SETFL, flags | O_NONBLOCK);
        setsockopt(NativeHandle, SOL_SOCKET, SO_NOSIGPIPE, (char *)X2Ptr(int(1)), sizeof(int));

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
        _WriteBufferPtr = nullptr;
        _Status = eStatus::Connected;
        _SuspendReading = false;
        _FlushFlag = false;
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
        _WriteBufferPtr = nullptr;
        _SuspendReading = false;
        _FlushFlag = false;

        FailSafe.Dismiss();
        SetAvailable();
        return true;
    }

    void xTcpConnection::Clean()
    {
        X_DEBUG_PRINTF("Cleaning tcp connection: %p\n", this);
        if (_WriteBufferPtr) {
            delete _WriteBufferPtr;
            while(auto WriteBufferPtr = _WriteBufferChain.Pop()) {
                delete WriteBufferPtr;
            }
        }
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
                X_DEBUG_PRINTF("xTcpConnection::OnIoEventInReady EOF\n");
                _Status = eStatus::Closing;
                _ListenerPtr->OnPeerClose(this);
                SetDisabled();
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
        // X_DEBUG_PRINTF("xTcpConnection::OnIoEventOutReady\n");
        if (_Status == eStatus::Connecting) {
            X_DEBUG_PRINTF("Connection established\n");
            _Status = eStatus::Connected;
            _ListenerPtr->OnConnected(this);
        } else {
            TrySendData();
        }
        if (Steal(_FlushFlag)) {
            _ListenerPtr->OnFlush(this);
        }
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
                        EnableWritingTrigger();
                    }
                    return;
                }
                SetError();
                return;
            }
            if ((_WriteBufferPtr->DataSize -= SendSize)) {
                memmove(_WriteBufferPtr->Buffer, _WriteBufferPtr->Buffer + SendSize, _WriteBufferPtr->DataSize);
                if (!_RequireOutputEvent) {
                    _RequireOutputEvent = true;
                    EnableWritingTrigger();
                }
                return;
            }
            delete _WriteBufferPtr;
            _WriteBufferPtr = _WriteBufferChain.Pop();
        }
        _FlushFlag = true;
        if (_RequireOutputEvent) {
            _RequireOutputEvent = false;
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
