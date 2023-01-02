/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include "support/network/sockets/bsd/SocketBsdCompat.h"
#include <support/include/support/logging/Log.h>

SocketInitializer::SocketInitializer() {
#ifdef _WIN32
    WSADATA WinsockData;
    int ret = WSAStartup(MAKEWORD(2, 2), &WinsockData);
    if (ret != 0) {
        HUE_LOG << HUE_NETWORK << HUE_ERROR << "SocketInitializer: WSAStartup error is " << ret << HUE_ENDL;
    }
#endif
}

SocketInitializer::~SocketInitializer() {
#ifdef _WIN32
    WSACleanup();
#endif
}

int socket_set_nonblocking(SOCKET_ID fd) {
#ifdef _WIN32
    u_long mode = 1;
    return ioctlsocket(fd, FIONBIO, &mode);
#else
    int flags = fcntl(fd, F_GETFL, 0);
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
#endif
}
