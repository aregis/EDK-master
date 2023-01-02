/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <gmock/gmock.h>

#include <memory>

#include "support/network/sockets/SocketUdp.h"
#include "support/network/sockets/_test/SocketUdpDelegator.h"
#include "support/network/sockets/SocketAddress.h"

using std::shared_ptr;
using support::SocketAddress;
using support::SocketUdp;
using support::SocketUdpDelegatorProvider;

namespace support_unittests {

    class MockSocketUdpDelegateProvider : public SocketUdpDelegatorProvider {
    public:
        /** 
        
         */
        MOCK_METHOD1(get_delegate, shared_ptr<SocketUdp>(const SocketAddress& address_local));
    };

}  // namespace support_unittests

