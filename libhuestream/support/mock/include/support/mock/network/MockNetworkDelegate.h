/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <gmock/gmock.h>

#include <vector>

#include "support/network/_test/NetworkDelegator.h"

using support::NetworkDelegate;
using support::NetworkInterface;
using support::NetworkAdapterType;

namespace support_unittests {

    class MockNetworkDelegate : public NetworkDelegate {
    public:   
        /** 
        
         */
        MockNetworkDelegate() {
            ON_CALL(*this, is_wifi_connected()).WillByDefault(testing::Invoke([]() {
                return true;
            }));
        }

        MOCK_METHOD0(get_network_interfaces, const std::vector<NetworkInterface>());

        MOCK_METHOD0(is_wifi_connected, bool());
    };

}  // namespace support_unittests

