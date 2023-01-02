/*******************************************************************************
 Copyright (C) 2018 Philips Lighting Holding B.V.
 All Rights Reserved.
 ********************************************************************************/

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <memory>

#include "bridgediscovery/BridgeDiscovery.h"
#include "method/BridgeDiscoveryMethodFactory.h"
#include "method/ipscan/BridgeDiscoveryIpscan.h"
#include "method/nupnp/BridgeDiscoveryNupnp.h"
#include "method/upnp/BridgeDiscoveryUpnp.h"

using std::shared_ptr;
using huesdk::BridgeDiscovery;
using huesdk::IBridgeDiscoveryMethod;
using huesdk::BRIDGE_DISCOVERY_CLASS_TYPE_IPSCAN;
using huesdk::BRIDGE_DISCOVERY_CLASS_TYPE_NUPNP;
using huesdk::BRIDGE_DISCOVERY_CLASS_TYPE_UPNP;

TEST(TestBridgeDiscoveryMethodProvider, GetDiscoveryMethod_UpnpOption__ReturnsDiscoveryMethod) {
    auto discovery_method
            = huesdk_lib_default_factory<huesdk::IBridgeDiscoveryMethod>(BridgeDiscovery::Option::UPNP);

    EXPECT_NE(nullptr, discovery_method.get());
    EXPECT_EQ(BRIDGE_DISCOVERY_CLASS_TYPE_UPNP, discovery_method->get_type());
}

TEST(TestBridgeDiscoveryMethodProvider, GetDiscoveryMethod_NupnpOption__ReturnsDiscoveryMethod) {
    auto discovery_method
            = huesdk_lib_default_factory<huesdk::IBridgeDiscoveryMethod>(BridgeDiscovery::Option::NUPNP);

    EXPECT_NE(nullptr, discovery_method.get());
    EXPECT_EQ(BRIDGE_DISCOVERY_CLASS_TYPE_NUPNP, discovery_method->get_type());
}

TEST(TestBridgeDiscoveryMethodProvider, GetDiscoveryMethod_IpscanOption__ReturnsDiscoveryMethod) {
    auto discovery_method
            = huesdk_lib_default_factory<huesdk::IBridgeDiscoveryMethod>(BridgeDiscovery::Option::IPSCAN);

    EXPECT_NE(nullptr, discovery_method.get());
    EXPECT_EQ(BRIDGE_DISCOVERY_CLASS_TYPE_IPSCAN, discovery_method->get_type());
}
