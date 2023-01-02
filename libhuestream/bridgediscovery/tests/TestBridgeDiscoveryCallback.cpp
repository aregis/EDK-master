/*******************************************************************************
 Copyright (C) 2018 Philips Lighting Holding B.V.
 All Rights Reserved.
 ********************************************************************************/
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <atomic>

#include "bridgediscovery/BridgeDiscoveryResult.h"
#include "bridgediscovery/IBridgeDiscoveryCallback.h"

using std::vector;

using huesdk::BridgeDiscoveryResult;
using huesdk::BridgeDiscoveryReturnCode;
using huesdk::BridgeDiscoveryCallback;
using huesdk::BRIDGE_DISCOVERY_RETURN_CODE_SUCCESS;

/** whether the callback is called when done searching */
std::atomic<bool> _callback_called(false);

TEST(TestBridgeDiscoveryCallback, CallbackLambda__LambdaCalledWithCallbackInput) {
    vector<std::shared_ptr<BridgeDiscoveryResult>> results;
    // Initialize Vector
    // Set some fake
    results.emplace_back(std::make_shared<huesdk::BridgeDiscoveryResult>("0017881055ec", "192.168.2.1", "1.21.0", "BSB002"));
    results.emplace_back(std::make_shared<huesdk::BridgeDiscoveryResult>("0017881055ed", "192.168.2.2", "1.21.0", "BSB002"));
    results.emplace_back(std::make_shared<huesdk::BridgeDiscoveryResult>("0017881055ee", "192.168.2.3", "1.21.0", "BSB002"));
    
    _callback_called = false;

    // Setup callback with lambda
    BridgeDiscoveryCallback callback([] (vector<std::shared_ptr<BridgeDiscoveryResult>> results, BridgeDiscoveryReturnCode return_code) {
        EXPECT_EQ(3ul,       results.size());
        EXPECT_EQ(BRIDGE_DISCOVERY_RETURN_CODE_SUCCESS, return_code);
        
        _callback_called = true;
    });
    
    // Call the callback
    callback(vector<std::shared_ptr<BridgeDiscoveryResult>>(results), BRIDGE_DISCOVERY_RETURN_CODE_SUCCESS);
    
    // Verify that the callback handler has been called
    EXPECT_TRUE(_callback_called.load());
}
