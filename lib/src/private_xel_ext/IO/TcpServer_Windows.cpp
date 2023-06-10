#include <xel_ext/IO/TcpServer.hpp>

#if defined(X_SYSTEM_WINDOWS)

#include <cinttypes>
X_NS
{

    bool xTcpServer::Init(xIoContext * IoContextPtr, const xNetAddress & Address, iListener * ListenerPtr, bool ReusePort)
    {
        assert(Address);
        _IoContextPtr = IoContextPtr;
        _ListenerPtr = ListenerPtr;

        sockaddr_storage AddrStorage;
        size_t AddrLen = Address.Dump(&AddrStorage);

        assert(_ListenSocket == InvalidSocket);
        _ListenSocket = InvalidSocket;

        assert(_PreAcceptSocket == InvalidSocket);
        _PreAcceptSocket = InvalidSocket;

        auto FailSafe = xScopeGuard{[&]{
            if (_ListenSocket != InvalidSocket) {
                XelCloseSocket(X_DEBUG_STEAL(_ListenSocket, InvalidSocket));
            }
            if (_PreAcceptSocket != InvalidSocket) {
                XelCloseSocket(X_DEBUG_STEAL(_PreAcceptSocket, InvalidSocket));
            }
        }};

        struct addrinfo * addrlocal = NULL;
        _AF = Address.IsV4() ? AF_INET : AF_INET6; // address family
        _ListenSocket = WSASocket(_AF, SOCK_STREAM, IPPROTO_IP, NULL, 0, WSA_FLAG_OVERLAPPED);
        if (_ListenSocket == InvalidSocket) {
            return false;
        }
        setsockopt(_ListenSocket, SOL_SOCKET, SO_SNDBUF, (char *)X2Ptr(int(0)), sizeof(int));

        if (ReusePort) {
        #ifdef SO_REUSEADDR
            setsockopt(_ListenSocket, SOL_SOCKET, SO_REUSEADDR, (char *)X2Ptr(int(1)), sizeof(int));
        #endif
        }

        auto BindRet = bind(_ListenSocket, (sockaddr*)&AddrStorage, (int)AddrLen);
        if( BindRet == SOCKET_ERROR ) {
            return false;
        }

        auto ListenRet = listen(_ListenSocket, SOMAXCONN);
        if( ListenRet == SOCKET_ERROR ) {
            X_DEBUG_PRINTF("xTcpServer::Init failed to listen: socket=%" PRIuPTR "\n", (uintptr_t)_ListenSocket);
            return false;
        }
        if (CreateIoCompletionPort((HANDLE)_ListenSocket, *IoContextPtr, (ULONG_PTR)this, 0) == NULL) {
            X_DEBUG_PRINTF("xTcpServer::TryPreAccept failed to create completion port\n");
            return false;
        }

        if (!(_IoBufferPtr = CreateOverlappedObject())) { return false; }
        auto OverlappedObjectGuard = xScopeGuard([this]{ ReleaseOverlappedObject(_IoBufferPtr); });

        OverlappedObjectGuard.Dismiss();
        FailSafe.Dismiss();
		SetAvailable();

        IoContextPtr->DeferCallback(*this);
        return true;
    }

    void xTcpServer::OnDeferredCallback()
    {
        if (!IsAvailable()) {
            return;
        }
        TryPreAccept();
        if (HasError()) {
            OnIoEventError();
        }
    }

    void xTcpServer::TryPreAccept()
    {
		auto & ReadObject = _IoBufferPtr->ReadObject;
		if (ReadObject.AsyncOpMark) {
            return;
        }

        assert(_PreAcceptSocket == InvalidSocket);
        _PreAcceptSocket = WSASocket(_AF, SOCK_STREAM, IPPROTO_IP, NULL, 0, WSA_FLAG_OVERLAPPED);
        if (_PreAcceptSocket == InvalidSocket) {
            X_DEBUG_PRINTF("xTcpServer::TryPreAccept failed to create pre accept socket\n");
            Fatal();
            return;
        }

        auto & Overlapped = ReadObject.NativeOverlappedObject;
        memset(&Overlapped, 0 , sizeof(Overlapped));
        auto AcceptResult = AcceptEx(_ListenSocket, _PreAcceptSocket, &_PreAcceptAddress, 0,
            sizeof(_PreAcceptAddress.Local) + 16,
            sizeof(_PreAcceptAddress.Remote) + 16, &_PreAcceptAddress._PreAcceptReceivedLength, &Overlapped);
        if (!AcceptResult && ERROR_IO_PENDING != WSAGetLastError()) {
            X_DEBUG_PRINTF("xTcpServer::TryPreAccept failed to exec AcceptEx: reason=%i\n", WSAGetLastError());
            SetError();
            return;
        }
        ReadObject.AsyncOpMark = true;
        RetainOverlappedObject(_IoBufferPtr);
    }

    void xTcpServer::OnIoEventInReady()
    {
		auto & ReadObject = _IoBufferPtr->ReadObject;
		ReadObject.AsyncOpMark = false;

        setsockopt(_PreAcceptSocket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char *)&_ListenSocket, sizeof(_ListenSocket));
        _ListenerPtr->OnNewConnection(this, Steal(_PreAcceptSocket, InvalidSocket));

        _IoContextPtr->DeferCallback(*this);
    }

    void xTcpServer::Clean()
    {
        assert(_ListenSocket != InvalidSocket);
        if (_PreAcceptSocket != InvalidSocket) {
            XelCloseSocket(X_DEBUG_STEAL(_PreAcceptSocket, InvalidSocket));
        }
        XelCloseSocket(X_DEBUG_STEAL(_ListenSocket, InvalidSocket));
        X_DEBUG_PRINTF("xTcpServer::Clean succeeded: Object=%p\n", this);

        X_DEBUG_RESET(_ListenerPtr);
        X_DEBUG_RESET(_IoContextPtr);
    }

}

#endif
