/*******************************************************************************
 Copyright (C) 2018 Philips Lighting Holding B.V.
 All Rights Reserved.
 ********************************************************************************/

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "bridgediscovery/BridgeDiscoveryResult.h"

using huesdk::BridgeDiscoveryResult;

TEST(TestBridgeDiscoveryResult, Constructor_WithUniqueIdString_AndIpString_AndApiVersion__GettersReturnSameAsInput) {
    BridgeDiscoveryResult result("fake_unique_id", "fake_ip", "fake_version", "fake_model_id");
    
    EXPECT_STREQ("fake_unique_id", result.get_unique_id());
    EXPECT_STREQ("fake_ip", result.get_ip());
    EXPECT_STREQ("fake_version", result.get_api_version());
    EXPECT_STREQ("fake_model_id", result.get_model_id());
}

TEST(TestBridgeDiscoveryResult, Constructor_WithEmpties__GettersReturnSameAsInput) {
    BridgeDiscoveryResult result {"", "", "", ""};

    EXPECT_STREQ("", result.get_unique_id());
    EXPECT_STREQ("", result.get_ip());
    EXPECT_STREQ("", result.get_api_version());
    EXPECT_STREQ("", result.get_model_id());
}
