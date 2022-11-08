#include <xel_ext/IO/TcpServer.hpp>

#if defined(X_SYSTEM_WINDOWS) 

#include <cinttypes>
X_NS
{
    
    bool xTcpServer::Init(xIoContext * IoContextPtr, const xNetAddress & Address, iListener * ListenerPtr, bool ReusePort)
    {
        assert(Address);

        sockaddr_storage AddrStorage;
        size_t AddrLen = 0;
        memset(&AddrStorage, 0, sizeof(AddrStorage));
        if (Address.IsV4()) {
            auto & Addr4 = (sockaddr_in&)AddrStorage;
            Addr4.sin_family = AF_INET;
            Addr4.sin_addr = (decltype(sockaddr_in::sin_addr)&)(Address.Ipv4);
            Addr4.sin_port = htons(Address.Port);
            AddrLen = sizeof(sockaddr_in);
        } else {
            auto & Addr6 = (sockaddr_in6&)AddrStorage;
            Addr6.sin6_family = AF_INET6;
            Addr6.sin6_addr = (decltype(sockaddr_in6::sin6_addr)&)(Address.Ipv6);
            Addr6.sin6_port = htons(Address.Port);
            AddrLen = sizeof(sockaddr_in6);
        }
        
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
        
        _IoContextPtr = IoContextPtr;
        _ListenerPtr = ListenerPtr;

        TryPreAccept();
        FailSafe.Dismiss();

        X_DEBUG_PRINTF("xTcpServer::Init succeeded: Object=%p\n", this);
        return true;
    }

    void xTcpServer::TryPreAccept()    
    {
        if (_PreAcceptSocket == InvalidSocket) {
            _PreAcceptSocket = WSASocket(_AF, SOCK_STREAM, IPPROTO_IP, NULL, 0, WSA_FLAG_OVERLAPPED);
            if (_PreAcceptSocket == InvalidSocket) {
                X_DEBUG_PRINTF("xTcpServer::TryPreAccept failed to create pre accept socket\n");
                _IoContextPtr->DeferOperation(*this);
                return;
            }
            setsockopt(_PreAcceptSocket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char *)&_ListenSocket, sizeof(_ListenSocket));
        }
        memset(&_Overlapped, 0 , sizeof(_Overlapped));
        auto AcceptResult = AcceptEx(_ListenSocket, _PreAcceptSocket, &_PreAcceptAddress, 0, sizeof(_PreAcceptAddress.Local), sizeof(_PreAcceptAddress.Remote), &_PreAcceptReceivedLength, &_Overlapped);
        if (!AcceptResult && ERROR_IO_PENDING != WSAGetLastError()) {
            X_DEBUG_PRINTF("xTcpServer::TryPreAccept failed to exec AcceptEx: reason=%i\n", WSAGetLastError());
            _IoContextPtr->DeferOperation(*this);
            return;
        }
    }

    void xTcpServer::OnIoEventOutReady()
    {
        _ListenerPtr->OnNewConnection(this, Steal(_PreAcceptSocket, InvalidSocket));
        TryPreAccept();
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
