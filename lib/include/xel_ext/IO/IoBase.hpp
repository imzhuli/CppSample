#pragma once
#include <xel/Common.hpp>

#if defined(X_SYSTEM_WINDOWS)
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
    #include <windef.h>
    #include <ws2def.h>
    #include <winsock2.h>
    #include <Ws2tcpip.h>

X_NS {
    typedef SSIZE_T                   ssize_t;
    typedef int                       send_len_t;
    typedef int                       recv_len_t;
    typedef HANDLE                    xEventPoller;
    typedef xVariable                 xNativeEventType;
    typedef SOCKET                    xSocket;
    X_API const xEventPoller          InvalidEventPoller; // INVALID_HANDLE_VALUE
    constexpr xSocket                 InvalidSocket = INVALID_SOCKET;
    #define XelCloseSocket(sockfd)    closesocket((sockfd))
}

#elif defined(X_SYSTEM_LINUX)
    #include <sys/epoll.h>
    #include <sys/socket.h>
    #include <sys/types.h>
    #include <unistd.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>

X_NS {
    typedef size_t                     send_len_t;
    typedef size_t                     recv_len_t;
    typedef int                        xEventPoller;        // epoll
    typedef enum EPOLL_EVENTS          xNativeEventType;    // EPOLLIN EPOLLOUT EPOLLERR ...
    typedef int                        xSocket;
    constexpr xEventPoller             InvalidEventPoller = ((xEventPoller)-1);
    constexpr xSocket                  InvalidSocket = ((xSocket)-1);
    #define XelCloseSocket(sockfd)     close((sockfd))
}

#elif defined(X_SYSTEM_DARWIN)
    #include <sys/event.h>
    #include <sys/socket.h>
    #include <sys/types.h>
    #include <unistd.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>

X_NS {
    typedef size_t                     send_len_t;
    typedef size_t                     recv_len_t;
    typedef int                        xEventPoller;        // kqueue
    typedef int                        xSocket;
    constexpr xEventPoller             InvalidEventPoller = ((xEventPoller)-1);
    constexpr xSocket                  InvalidSocket = ((xSocket)-1);
    #define XelCloseSocket(sockfd)     close((sockfd))
}

#else
    #error unsupported system type
#endif

#ifdef MSG_NOSIGNAL
    #define XelNoWriteSignal       MSG_NOSIGNAL
#else
    #define XelNoWriteSignal       0
#endif
#ifndef SOCK_CLOEXEC
#define SOCK_CLOEXEC               0
#endif
