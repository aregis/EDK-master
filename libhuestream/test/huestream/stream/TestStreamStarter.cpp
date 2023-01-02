/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <huestream/stream/StreamStarter.h>
#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "test/huestream/_mock/MockBridgeHttpClient.h"

using ::testing::Return;
using ::testing::_;
using ::testing::Sequence;
using ::testing::Property;
using ::testing::AllOf;
using ::testing::Pointee;
using ::testing::InSequence;

namespace huestream {

    class TestStreamStarter : public testing::Test,
                              public testing::WithParamInterface<bool> {
    public:
        BridgePtr _bridge;
        std::shared_ptr<MockBridgeHttpClient> _mockHttpPtr;
        std::shared_ptr<StreamStarter> _streamStarter;

        virtual void SetUp() {
            _bridge = CreateBridge();
            _mockHttpPtr = std::make_shared<MockBridgeHttpClient>();
            _streamStarter = std::make_shared<StreamStarter>(_bridge, _mockHttpPtr);
        }

        virtual void TearDown() {
        }

        BridgePtr CreateBridge() {
            auto group1 = std::make_shared<Group>();
            group1->SetId("1");
            auto group2 = std::make_shared<Group>();
            group2->SetId("2");
            auto groupList = std::make_shared<GroupList>();
            groupList->push_back(group1);
            groupList->push_back(group2);
            auto bridge = std::make_shared<Bridge>(std::make_shared<BridgeSettings>());
            bridge->SetGroups(groupList);
            bridge->SetSelectedGroup("2");
            bridge->SetIpAddress("192.168.1.15");
            bridge->SetUser("8932746jhb23476");
            bridge->SetIsValidIp(true);
            bridge->SetIsAuthorized(true);
            bridge->SetId("SOMEID");
            bridge->SetModelId("BSB002");

            bridge->SetClientKey("DD129216F1A50E5D1C0CB356325745F2");
            bridge->SetIsBusy(false);
            return bridge;
        }

        std::string GetProtocol() {
            return "https";
        }
    };

    static HttpRequestPtr GetStreamActiveResponseSuccess() {
        auto req = std::make_shared<HttpRequestInfo>();
        req->SetSuccess(true);
        req->SetResponse("[{\"success\":{\"/groups/2/stream/active\":true}}]");
        return req;
    }

    static HttpRequestPtr GetStreamDeactivateGroupTwoResponseSuccess() {
        auto req = std::make_shared<HttpRequestInfo>();
        req->SetSuccess(true);
        req->SetResponse("[{\"success\":{\"/groups/2/stream/active\":false}}]");
        return req;
    }

    static HttpRequestPtr GetStreamDeactivateGroupOneResponseSuccess() {
        auto req = std::make_shared<HttpRequestInfo>();
        req->SetSuccess(true);
        req->SetResponse("[{\"success\":{\"/groups/1/stream/active\":false}}]");
        return req;
    }

    static HttpRequestPtr GetStreamActiveResponseErrorNoResponse() {
        auto req = std::make_shared<HttpRequestInfo>();
        req->SetSuccess(false);
        return req;
    }

    static HttpRequestPtr GetStreamActiveResponseErrorCantActivate() {
        auto req = std::make_shared<HttpRequestInfo>();
        req->SetSuccess(true);
        req->SetResponse(
            "[{\"error\":{\"type\":307,\"address\":\"/groups/2/stream/active\",\"description\":\"Cannot claim stream ownership\"}}]");
        return req;
    }

    static HttpRequestPtr GetStreamActiveResponseErrorNotAuthorized() {
        auto req = std::make_shared<HttpRequestInfo>();
        req->SetSuccess(true);
        req->SetResponse(
            "[{\"error\":{\"type\":1,\"address\":\"/groups/2/stream/active\",\"description\":\"unauthorized user\"}}]");
        return req;
    }

    static HttpRequestPtr GetStreamActiveResponseErrorInvalidGroup() {
        auto req = std::make_shared<HttpRequestInfo>();
        req->SetSuccess(true);
        req->SetResponse(
            "[{\"error\":{\"type\":3,\"address\":\"/groups/2/stream/active\",\"description\":\"resource not available\"}}]");
        return req;
    }

    static HttpRequestPtr GetStreamDeactivateResponseSuccess() {
        auto req = std::make_shared<HttpRequestInfo>();
        req->SetSuccess(true);
        req->SetResponse("[{\"success\":{\"/groups/2/stream/active\":false}}]");
        return req;
    }

    static HttpRequestPtr GetStreamDeactivateResponseErrorInvalidGroup() {
        auto req = std::make_shared<HttpRequestInfo>();
        req->SetSuccess(false);
        req->SetResponse(
            "[{\"error\":{\"type\":3,\"address\":\"/groups/3/stream/active\",\"description\":\"resource not available\"}}]");
        return req;
    }

    INSTANTIATE_TEST_CASE_P(HttpAndHttps, TestStreamStarter, testing::Values(true, false));

    TEST_P(TestStreamStarter, StartSuccess) {
        std::string body = "{\"stream\":{\"active\":true}}";
        std::string url = GetProtocol() + "://192.168.1.15/api/8932746jhb23476/groups/2";
        EXPECT_CALL(*_mockHttpPtr, ExecuteHttpRequest(_, HTTP_REQUEST_PUT, url, body, false)).Times(1).WillOnce(
                Return(GetStreamActiveResponseSuccess()));
        auto success = _streamStarter->Start(false);
        ASSERT_TRUE(success);
        ASSERT_FALSE(_bridge->IsBusy());
        ASSERT_TRUE(_bridge->GetGroup()->Active());
        ASSERT_EQ(_bridge->GetGroup()->GetOwner(), _bridge->GetUser());
    }

    TEST_P(TestStreamStarter, StartUnforcedFail) {
        std::string body = "{\"stream\":{\"active\":true}}";
        std::string url = GetProtocol() + "://192.168.1.15/api/8932746jhb23476/groups/2";
        EXPECT_CALL(*_mockHttpPtr, ExecuteHttpRequest(_, HTTP_REQUEST_PUT, url, body, false)).Times(1).WillOnce(
                Return(GetStreamActiveResponseErrorCantActivate()));
        auto success = _streamStarter->Start(false);
        ASSERT_FALSE(success);
        ASSERT_TRUE(_bridge->IsBusy());
    }

    TEST_P(TestStreamStarter, StartForcedSuccess) {
        std::string url = GetProtocol() + "://192.168.1.15/api/8932746jhb23476/groups/2";
        std::string bodyStart = "{\"stream\":{\"active\":true}}";
        std::string bodyStop = "{\"stream\":{\"active\":false}}";

        {
            InSequence dummy;
            EXPECT_CALL(*_mockHttpPtr, ExecuteHttpRequest(_, HTTP_REQUEST_PUT, url, bodyStart, false)).Times(1).WillOnce(
                    Return(GetStreamActiveResponseErrorCantActivate()));
            EXPECT_CALL(*_mockHttpPtr, ExecuteHttpRequest(_, HTTP_REQUEST_PUT, url, bodyStop, false)).Times(1).WillOnce(Return(
                    std::make_shared<HttpRequestInfo>()));
            EXPECT_CALL(*_mockHttpPtr, ExecuteHttpRequest(_, HTTP_REQUEST_PUT, url, bodyStart, false)).Times(1).WillOnce(
                    Return(GetStreamActiveResponseSuccess()));
        }

        auto success = _streamStarter->Start(true);
        ASSERT_TRUE(success);
        ASSERT_FALSE(_bridge->IsBusy());
        ASSERT_TRUE(_bridge->GetGroup()->Active());
    }

    TEST_P(TestStreamStarter, StartFailNoResponse) {
        std::string body = "{\"stream\":{\"active\":true}}";
        std::string url = GetProtocol() + "://192.168.1.15/api/8932746jhb23476/groups/2";
        EXPECT_CALL(*_mockHttpPtr, ExecuteHttpRequest(_, HTTP_REQUEST_PUT, url, body, false)).Times(1).WillOnce(
            Return(GetStreamActiveResponseErrorNoResponse()));
        auto success = _streamStarter->Start(true);
        ASSERT_FALSE(success);
        ASSERT_FALSE(_bridge->IsValidIp());
    }

    TEST_P(TestStreamStarter, StartFailNotAuthorized) {
        std::string body = "{\"stream\":{\"active\":true}}";
        std::string url = GetProtocol() + "://192.168.1.15/api/8932746jhb23476/groups/2";
        EXPECT_CALL(*_mockHttpPtr, ExecuteHttpRequest(_, HTTP_REQUEST_PUT, url, body, false)).Times(1).WillOnce(
            Return(GetStreamActiveResponseErrorNotAuthorized()));
        auto success = _streamStarter->Start(false);
        ASSERT_FALSE(success);
        ASSERT_FALSE(_bridge->IsAuthorized());
    }

    TEST_P(TestStreamStarter, StartFailInvalidGroup) {
        std::string body = "{\"stream\":{\"active\":true}}";
        std::string url = GetProtocol() + "://192.168.1.15/api/8932746jhb23476/groups/2";
        EXPECT_CALL(*_mockHttpPtr, ExecuteHttpRequest(_, HTTP_REQUEST_PUT, url, body, false)).Times(1).WillOnce(
            Return(GetStreamActiveResponseErrorInvalidGroup()));
        auto success = _streamStarter->Start(true);
        ASSERT_FALSE(success);
        ASSERT_FALSE(_bridge->IsValidGroupSelected());
    }

    TEST_P(TestStreamStarter, Stop) {
        _bridge->GetGroup()->SetActive(true);
        _bridge->GetGroup()->SetOwner(_bridge->GetUser());
        std::string body = "{\"stream\":{\"active\":false}}";
        std::string url = GetProtocol() + "://192.168.1.15/api/8932746jhb23476/groups/2";
        EXPECT_CALL(*_mockHttpPtr, ExecuteHttpRequest(_, HTTP_REQUEST_PUT, url, body, false)).Times(1).WillOnce(
            Return(GetStreamDeactivateResponseSuccess()));
        _streamStarter->Stop();
        ASSERT_FALSE(_bridge->GetGroup()->Active());
        ASSERT_TRUE(_bridge->GetGroup()->GetOwner().empty());
    }

    TEST_P(TestStreamStarter, DeactivateOtherGroup) {
        _bridge->GetGroupById("1")->SetActive(true);
        _bridge->GetGroupById("1")->SetOwner("someone");
        std::string body = "{\"stream\":{\"active\":false}}";
        std::string url = GetProtocol() + "://192.168.1.15/api/8932746jhb23476/groups/1";
        EXPECT_CALL(*_mockHttpPtr, ExecuteHttpRequest(_, HTTP_REQUEST_PUT, url, body, false)).Times(1).WillOnce(
            Return(GetStreamDeactivateResponseSuccess()));
        ASSERT_TRUE(_streamStarter->DeactivateGroup("1"));
        ASSERT_FALSE(_bridge->GetGroupById("1")->Active());
        ASSERT_TRUE(_bridge->GetGroupById("1")->GetOwner().empty());
    }

    TEST_P(TestStreamStarter, DeactivateWrongGroup) {
        std::string body = "{\"stream\":{\"active\":false}}";
        std::string url = GetProtocol() + "://192.168.1.15/api/8932746jhb23476/groups/3";
        EXPECT_CALL(*_mockHttpPtr, ExecuteHttpRequest(_, HTTP_REQUEST_PUT, url, body, false)).Times(1).WillOnce(
            Return(GetStreamDeactivateResponseErrorInvalidGroup()));
        ASSERT_FALSE(_streamStarter->DeactivateGroup("3"));
    }

    TEST_P(TestStreamStarter, StartOverrideAlwaysSuccess) {
        _bridge->GetGroupById("1")->SetActive(true);
        _bridge->GetGroupById("1")->SetOwner("someone");

        std::string url = GetProtocol() + "://192.168.1.15/api/8932746jhb23476/groups/2";
        std::string urlOther = GetProtocol() + "://192.168.1.15/api/8932746jhb23476/groups/1";
        std::string bodyStart = "{\"stream\":{\"active\":true}}";
        std::string bodyStop = "{\"stream\":{\"active\":false}}";

        {
            InSequence dummy;
            EXPECT_CALL(*_mockHttpPtr, ExecuteHttpRequest(_, HTTP_REQUEST_PUT, url, bodyStart, false)).Times(1).WillOnce(
                Return(GetStreamActiveResponseErrorCantActivate()));
            EXPECT_CALL(*_mockHttpPtr, ExecuteHttpRequest(_, HTTP_REQUEST_PUT, urlOther, bodyStop, false)).Times(1).WillOnce(
                Return(GetStreamDeactivateGroupOneResponseSuccess()));
            EXPECT_CALL(*_mockHttpPtr, ExecuteHttpRequest(_, HTTP_REQUEST_PUT, url, bodyStop, false)).Times(1).WillOnce(Return(std::make_shared<HttpRequestInfo>()));
            EXPECT_CALL(*_mockHttpPtr, ExecuteHttpRequest(_, HTTP_REQUEST_PUT, url, bodyStart, false)).Times(1).WillOnce(
                Return(GetStreamActiveResponseSuccess()));
        }

        auto success = _streamStarter->StartStream(ACTIVATION_OVERRIDELEVEL_ALWAYS);
        ASSERT_TRUE(success);
        ASSERT_FALSE(_bridge->IsBusy());
        ASSERT_FALSE(_bridge->GetGroupById("1")->Active());
        ASSERT_TRUE(_bridge->GetGroup()->Active());
    }

}

