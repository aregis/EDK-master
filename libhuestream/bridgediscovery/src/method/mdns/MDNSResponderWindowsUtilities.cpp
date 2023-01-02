/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <winsock2.h>
#include <Ws2tcpip.h>

#include "method/mdns/MDNSResponderPlatformUtilities.h"
#include "mdns_responder/mDNSWindows/mDNSWin32.h"
#include "mdns_responder/mDNSShared/dns_sd.h"

using huesdk::MDNSResponderPlatformUtilities;


void MDNSResponderPlatformUtilities::init_mdns(mDNS *const m) {
    InterfaceListDidChange(m);
}

std::shared_ptr<mDNS_PlatformSupport> MDNSResponderPlatformUtilities::create_mdns_platform_support() {
    return std::unique_ptr<mDNS_PlatformSupport>{new mDNS_PlatformSupport{}};
}

extern "C" void DNSSD_API UDPSocketNotification(SOCKET sock, LPWSANETWORKEVENTS event, void *context);

static void mDNSAddToFDSet(int *nfds, fd_set *readfds, SOCKET s) {
    if (*nfds < s + 1) *nfds = static_cast<int>(s) + 1;
    FD_SET(s, readfds);
}

void MDNSResponderPlatformUtilities::get_fd_set(mDNS *const inMDNS, int *nfds, fd_set *readfds, struct timeval *timeout) {
    mDNSs32 ticks;
    struct timeval interval;

    // Call mDNS_Execute() to let mDNSCore do what it needs to do
    mDNSs32 nextevent = mDNS_Execute(inMDNS);

    // Build our list of active file descriptors
    mDNSInterfaceData *info = static_cast<mDNSInterfaceData*>(inMDNS->p->interfaceList);
    if (IsValidSocket(inMDNS->p->unicastSock4.fd)) mDNSAddToFDSet(nfds, readfds, inMDNS->p->unicastSock4.fd);
    if (IsValidSocket(inMDNS->p->unicastSock6.fd)) mDNSAddToFDSet(nfds, readfds, inMDNS->p->unicastSock6.fd);
    while (info) {
        if (IsValidSocket(info->sock.fd)) mDNSAddToFDSet(nfds, readfds, info->sock.fd);
        info = static_cast<mDNSInterfaceData*>(info->next);
    }

    // 3. Calculate the time remaining to the next scheduled event (in struct timeval format)
    ticks = nextevent - mDNS_TimeNow(inMDNS);
    if (ticks < 1) ticks = 1;
    interval.tv_sec  = ticks >> 10;                     // The high 22 bits are seconds
    interval.tv_usec = ((ticks & 0x3FF) * 15625) / 16;  // The low 10 bits are 1024ths

    // 4. If client's proposed timeout is more than what we want, then reduce it
    if (timeout->tv_sec > interval.tv_sec ||
        (timeout->tv_sec == interval.tv_sec && timeout->tv_usec > interval.tv_usec))
        *timeout = interval;
}

void MDNSResponderPlatformUtilities::process_fd_set(mDNS *const inMDNS, fd_set *readfds) {
    mDNSInterfaceData *info;
    info = static_cast<mDNSInterfaceData*>(inMDNS->p->interfaceList);

    if (IsValidSocket(inMDNS->p->unicastSock4.fd) && FD_ISSET(inMDNS->p->unicastSock4.fd, readfds)) {
        FD_CLR(inMDNS->p->unicastSock4.fd, readfds);
        UDPSocketNotification(inMDNS->p->unicastSock4.fd, NULL, &inMDNS->p->unicastSock4);
    }
    if (IsValidSocket(inMDNS->p->unicastSock6.fd) && FD_ISSET(inMDNS->p->unicastSock6.fd, readfds)) {
        FD_CLR(inMDNS->p->unicastSock6.fd, readfds);
        UDPSocketNotification(inMDNS->p->unicastSock6.fd, NULL, &inMDNS->p->unicastSock6);
    }

    while (info) {
        if (IsValidSocket(info->sock.fd) && FD_ISSET(info->sock.fd, readfds)) {
            FD_CLR(info->sock.fd, readfds);
            UDPSocketNotification(info->sock.fd, NULL, &info->sock);
        }
        info = static_cast<mDNSInterfaceData*>(info->next);
    }
}