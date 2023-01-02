/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <memory>

#include "support/network/sockets/bsd/SocketBsd.h"
#include "support/network/sockets/bsd/SocketBsdCompat.h"

#undef interface
#include "mdns_responder/mDNSCore/mDNSEmbeddedAPI.h"

namespace huesdk {
    class MDNSResponderPlatformUtilities {
    public:
        static void init_mdns(mDNS *const m);
        static std::shared_ptr<mDNS_PlatformSupport> create_mdns_platform_support();
        static void get_fd_set(mDNS *const m, int *nfds, fd_set *readfds, struct timeval *timeout);
        static void process_fd_set(mDNS *const m, fd_set* rset);
    };
}  // namespace huesdk