/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <memory>
#include <huestream/connect/BasicGroupLightController.h>
#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "test/huestream/_mock/MockBridgeHttpClient.h"

#include "support/network/http/HttpResponse.h"

using ::testing::Invoke;
using ::testing::Eq;
using ::testing::DoubleEq;
using ::testing::_;
using ::testing::Return;
using ::testing::NiceMock;

namespace huestream {

    class TestBasicGroupLightController : public testing::Test {
    public:
        BridgePtr _bridge;
        std::shared_ptr<MockBridgeHttpClient> _mockHttpClientPtr;
        std::shared_ptr<BasicGroupLightController> _groupController;
        const std::string actionUrl = "https://192.168.1.15/api/8932746jhb23476/groups/1/action";

        virtual void SetUp() {
            _bridge = std::make_shared<Bridge>(std::make_shared<BridgeSettings>());
            _bridge->SetId("11223344");
            _bridge->SetModelId("BSB002");
            _bridge->SetApiversion("1.24.0");
						_bridge->SetSwversion("1940094000");
            _bridge->SetIpAddress("192.168.1.15");
            _bridge->SetIsValidIp(true);
            _bridge->SetUser("8932746jhb23476");
            _bridge->SetIsAuthorized(true);
            _bridge->SetClientKey("nnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnn");
            auto group = std::make_shared<Group>();
            group->SetId("1");
            auto groups = std::make_shared<GroupList>();
            groups->push_back(group);
            _bridge->SetGroups(groups);
            ASSERT_TRUE(_bridge->SelectGroup("1"));

            _mockHttpClientPtr = std::make_shared<MockBridgeHttpClient>();

            _groupController = std::make_shared<BasicGroupLightController>(_mockHttpClientPtr);
            _groupController->SetActiveBridge(_bridge);
        }

        virtual void TearDown() {
        }

    };

    TEST_F(TestBasicGroupLightController, DontSetWhenInvalidGroup) {
        _bridge->SetSelectedGroup("30");
        ASSERT_FALSE(_bridge->IsValidGroupSelected());
        EXPECT_CALL(*_mockHttpClientPtr, ExecuteHttpRequest(_, _, _, _, _)).Times(0);
        _groupController->SetOn(true);
        _groupController->SetBrightness(0.3);
        _groupController->SetColor(0.8, 0.8);
        _groupController->SetPreset(0.8, 0.5, 0.7);
    }

    TEST_F(TestBasicGroupLightController, SetOn) {
        std::string body = "{\"on\":true}";
				EXPECT_CALL(*_mockHttpClientPtr, ExecuteHttpRequest(_, HTTP_REQUEST_PUT, actionUrl, body, _)).Times(1).WillOnce(Return(nullptr));

        _groupController->SetOn(true);
    }

    TEST_F(TestBasicGroupLightController, SetOff) {
        std::string body = "{\"on\":false}";
        EXPECT_CALL(*_mockHttpClientPtr, ExecuteHttpRequest(_, HTTP_REQUEST_PUT, actionUrl, body, _)).Times(1).WillOnce(Return(nullptr));

        _groupController->SetOn(false);
    }

    TEST_F(TestBasicGroupLightController, SetBri) {
        std::string body = "{\"bri\":51}";
        EXPECT_CALL(*_mockHttpClientPtr, ExecuteHttpRequest(_, HTTP_REQUEST_PUT, actionUrl, body, _)).Times(1).WillOnce(Return(nullptr));

        _groupController->SetBrightness(0.2);
    }

    TEST_F(TestBasicGroupLightController, SetColor) {
        std::string body = "{\"xy\":[0.1234,0.5678]}";
        EXPECT_CALL(*_mockHttpClientPtr, ExecuteHttpRequest(_, HTTP_REQUEST_PUT, actionUrl, body, _)).Times(1).WillOnce(Return(nullptr));

        _groupController->SetColor(0.1234, 0.5678);
    }

    TEST_F(TestBasicGroupLightController, SetPresetDefault) {
        std::string body = "{\"on\":true,\"bri\":254,\"xy\":[0.445,0.4067]}";
        EXPECT_CALL(*_mockHttpClientPtr, ExecuteHttpRequest(_, HTTP_REQUEST_PUT, actionUrl, body, _)).Times(1).WillOnce(Return(nullptr));

        _groupController->SetPreset(BasicGroupLightController::LIGHT_PRESET_READ);
    }

    TEST_F(TestBasicGroupLightController, SetPresetDefaultExcludeOff) {
        std::string body = "{\"bri\":254,\"xy\":[0.445,0.4067]}";
        EXPECT_CALL(*_mockHttpClientPtr, ExecuteHttpRequest(_, HTTP_REQUEST_PUT, actionUrl, body, _)).Times(1).WillOnce(Return(nullptr));

        _groupController->SetPreset(BasicGroupLightController::LIGHT_PRESET_READ, true);
    }

    TEST_F(TestBasicGroupLightController, SetPresetCustom) {
        std::string body = "{\"on\":true,\"bri\":127,\"xy\":[0.6,0.7]}";
        EXPECT_CALL(*_mockHttpClientPtr, ExecuteHttpRequest(_, HTTP_REQUEST_PUT, actionUrl, body, _)).Times(1).WillOnce(Return(nullptr));

        _groupController->SetPreset(0.5, 0.6, 0.7);
    }

    TEST_F(TestBasicGroupLightController, SetPresetCustomExcludeOff) {
        std::string body = "{\"bri\":127,\"xy\":[0.6,0.7]}";
        EXPECT_CALL(*_mockHttpClientPtr, ExecuteHttpRequest(_, HTTP_REQUEST_PUT, actionUrl, body, _)).Times(1).WillOnce(Return(nullptr));

        _groupController->SetPreset(0.5, 0.6, 0.7, true);
    }

    TEST_F(TestBasicGroupLightController, SetScene) {
        std::string body = "{\"scene\":\"1234abcd\"}";
        EXPECT_CALL(*_mockHttpClientPtr, ExecuteHttpRequest(_, HTTP_REQUEST_PUT, actionUrl, body, _)).Times(1).WillOnce(Return(nullptr));

        _groupController->SetScene("1234abcd");
    }

}

