/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

/*
    Minimal compatibility layer that unifies the socket APIs for Windows and Unix
*/

#ifdef _WIN32
    // Enable InetPton on mingw
    // 0x0600 = Windows Vista and above
    #ifdef _WIN32_WINNT
    #undef _WIN32_WINNT
    #endif
    #define _WIN32_WINNT 0x0600

    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <windows.h>

    #define SOCKET_ID         SOCKET
    #define SOCKET_INPROGRESS WSAEWOULDBLOCK
    #define SOCKET_ERRNO      (WSAGetLastError())
    #define inet_pton         InetPtonA

    #define getsockopt_(s, level, optname, optval, optlen) getsockopt(s, level, optname, reinterpret_cast<char*>(optval), optlen)
    #define setsockopt_(s, level, optname, optval, optlen) setsockopt(s, level, optname, reinterpret_cast<const char*>(optval), optlen)

#elif defined(_NX)

#include <nn/os.h>
#include <nn/socket.h>
#define SOCKET_ID         int
#define INVALID_SOCKET    -1
#define SOCKET_INPROGRESS EINPROGRESS

#define getsockopt_(s, level, optname, optval, optlen) nn::socket::GetSockOpt(s, level, optname, reinterpret_cast<char*>(optval), optlen)
#define setsockopt_(s, level, optname, optval, optlen) nn::socket::SetSockOpt(s, level, optname, reinterpret_cast<const char*>(optval), optlen)

#else
    #include <arpa/inet.h>
    #include <errno.h>
    #include <fcntl.h>
    #include <netinet/in.h>
    #include <sys/select.h>
    #include <sys/socket.h>
    #include <unistd.h>

    #define SOCKET_ID         int
    #define SOCKET_INPROGRESS EINPROGRESS
    #define SOCKET_ERRNO      errno
    #define INVALID_SOCKET    -1

    #define closesocket       close

    #define getsockopt_       getsockopt
    #define setsockopt_       setsockopt
#endif


class SocketInitializer {
public:
    static void Init() {
        static SocketInitializer instance;
    }

private:
    SocketInitializer();
    ~SocketInitializer();
};

int socket_set_nonblocking(SOCKET_ID fd);
