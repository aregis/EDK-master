/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include "method/mdns/MDNSResponderPlatformUtilities.h"

#include "mdns_responder/mDNSPosix/mDNSPosix.h"

using huesdk::MDNSResponderPlatformUtilities;

void MDNSResponderPlatformUtilities::init_mdns(mDNS *const m) {
    (void) m;
}

std::shared_ptr<mDNS_PlatformSupport> MDNSResponderPlatformUtilities::create_mdns_platform_support() {
    return std::unique_ptr<mDNS_PlatformSupport>{new mDNS_PlatformSupport{}};
}

void MDNSResponderPlatformUtilities::get_fd_set(mDNS *const m, int *nfds, fd_set *readfds, struct timeval *timeout) {
    mDNSPosixGetFDSet(m, nfds, readfds, timeout);
}

void MDNSResponderPlatformUtilities::process_fd_set(mDNS *const m, fd_set* rset) {
    mDNSPosixProcessFDSet(m, rset);
}