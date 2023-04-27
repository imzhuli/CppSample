#include <xel_ext/IO/TcpServer.hpp>

#if defined(X_SYSTEM_LINUX)
#include <fcntl.h>
#include <cinttypes>

#define X_ENABLE_REUSEPORT SO_REUSEPORT
#include <sys/socket.h>
#if defined(SO_REUSEPORT_LB)
#undef  X_ENABLE_REUSEPORT
#define X_ENABLE_REUSEPORT SO_REUSEPORT_LB
#endif

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

        _AF = Address.IsV4() ? AF_INET : AF_INET6; // address family
        _ListenSocket = socket(_AF, SOCK_STREAM, 0);
        if (_ListenSocket == InvalidSocket) {
            X_DEBUG_PRINTF("xTcpServer::Init failed to create listen socket\n");
            return false;
        }

        if (ReusePort) {
        #ifdef X_ENABLE_REUSEPORT
            int one = 1;
            setsockopt(_ListenSocket, SOL_SOCKET, X_ENABLE_REUSEPORT, &one, sizeof(one));
        #endif
        }

        int flags = fcntl(_ListenSocket, F_GETFL);
        fcntl(_ListenSocket, F_SETFL, flags | O_NONBLOCK);

        auto FailSafe = xScopeGuard{[&]{
            XelCloseSocket(X_DEBUG_STEAL(_ListenSocket, InvalidSocket));
        }};

        auto BindRet = bind(_ListenSocket, (sockaddr*)&AddrStorage, (int)AddrLen);
        if( BindRet == -1) {
            X_DEBUG_PRINTF("xTcpServer::Init failed bind: socket=%" PRIuPTR "\n", (uintptr_t)_ListenSocket);
            return false;
        }

        auto ListenRet = listen(_ListenSocket, SOMAXCONN);
        if( ListenRet == -1) {
            X_DEBUG_PRINTF("xTcpServer::Init failed to listen: socket=%" PRIuPTR "\n", (uintptr_t)_ListenSocket);
            return false;
        }

        struct epoll_event Event = {};
        Event.data.ptr = this;
        Event.events = EPOLLIN;
        if (epoll_ctl(*IoContextPtr, EPOLL_CTL_ADD, _ListenSocket, &Event)) {
            X_DEBUG_PRINTF("xTcpServer::Init failed: reason=unable_to_add_to_kqueue socket=%" PRIuPTR "\n", (uintptr_t)_ListenSocket);
            return false;
        }

        _IoContextPtr = IoContextPtr;
        _ListenerPtr = ListenerPtr;

        FailSafe.Dismiss();
        X_DEBUG_PRINTF("xTcpServer::Init succeeded: Object=%p\n", this);
        return true;
    }

    void xTcpServer::Clean()
    {
        assert(_ListenSocket != InvalidSocket);

        XelCloseSocket(X_DEBUG_STEAL(_ListenSocket, InvalidSocket));
        X_DEBUG_RESET(_ListenerPtr);
        X_DEBUG_RESET(_IoContextPtr);
    }

    void xTcpServer::OnIoEventInReady()
    {
        sockaddr_storage SockAddr = {};
        socklen_t SockAddrLen = sizeof(SockAddr);
        int NewSocket = accept(_ListenSocket, (struct sockaddr*)&SockAddr, &SockAddrLen);
        X_DEBUG_PRINTF("xTcpServer::OnIoEventInReady succeeded: NewSocket=%i\n", NewSocket);
        if (NewSocket == InvalidSocket) {
            return;
        }
        _ListenerPtr->OnNewConnection(this, NewSocket);
    }

}

#endif
