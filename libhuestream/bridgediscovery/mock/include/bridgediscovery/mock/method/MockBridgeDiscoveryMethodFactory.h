/*******************************************************************************
 Copyright (C) 2018 Philips Lighting Holding B.V.
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <gmock/gmock.h>

#include <memory>

#include "bridgediscovery/BridgeDiscovery.h"
#include "method/BridgeDiscoveryMethodFactory.h"

#include "method/IBridgeDiscoveryMethod.h"

using huesdk::IBridgeDiscoveryMethod;
using huesdk::BridgeDiscovery;

namespace support_unittests {

    class MockBridgeDiscoveryMethodFactory {
    public:
        std::unique_ptr<IBridgeDiscoveryMethod> do_create(BridgeDiscovery::Option option) {
            return std::unique_ptr<IBridgeDiscoveryMethod>(get_discovery_method_mock(option));
        }

        MOCK_METHOD1(get_discovery_method_mock, IBridgeDiscoveryMethod*(BridgeDiscovery::Option option));
    };

}  // namespace support_unittests

