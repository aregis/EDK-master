/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <memory>

#include "huestream/common/http/HttpClient.h"
#include "huestream/common/http/BridgeHttpClient.h"
#include "support/mock/network/http/MockHttpRequest.h"
#include "support/mock/network/http/MockHttpRequestDelegateProvider.h"

using huestream::HttpClient;
using huestream::BridgeHttpClient;
using huestream::BridgeHttpClientPtr;
using huestream::Bridge;
using huestream::BridgePtr;
using huestream::BridgeSettings;

using support_unittests::MockHttpRequest;
using support_unittests::MockHttpRequestDelegateProvider;

using support::HttpRequestDelegator;
using support::HttpResponse;
using support::HttpRequestError;

using testing::NiceMock;
using testing::StrictMock;
using testing::Invoke;
using testing::WithArg;
using testing::InvokeArgument;
using testing::DoAll;
using testing::AtLeast;

static constexpr auto BRIDGE_HTTPS_VERSION = "1.24.0";
static constexpr auto BRIDGE_SW_VERSION = "1940094000";

class TestBridgeHttpClient : public testing::Test {
public:
    void SetUp() override {
        _httpClient = std::make_shared<BridgeHttpClient>(std::make_shared<HttpClient>());
        _http_request_delegate_provider = std::make_shared<testing::NiceMock<MockHttpRequestDelegateProvider>>();
        HttpRequestDelegator::set_delegate_provider(_http_request_delegate_provider);
    }

    void TearDown() override {
        HttpRequestDelegator::set_delegate_provider({});
    }

    BridgePtr create_empty_https_bridge() {
        auto bridge =  std::make_shared<Bridge>(std::make_shared<BridgeSettings>());
        bridge->SetApiversion(BRIDGE_HTTPS_VERSION);
				bridge->SetSwversion(BRIDGE_SW_VERSION);
        return bridge;
    }

    shared_ptr<MockHttpRequest> create_mock_request(const std::string& url) {
        shared_ptr<MockHttpRequest> http_request= shared_ptr<MockHttpRequest>(
                new testing::NiceMock<MockHttpRequest>(url, 0));
        EXPECT_CALL(*http_request, fake_response()).WillRepeatedly(Return(HttpResponse(200, "")));
        EXPECT_CALL(*_http_request_delegate_provider, get_delegate(url, _, _, _, _, _))
                .WillOnce(Return(http_request));

        return http_request;
    }

protected:
    BridgeHttpClientPtr _httpClient;
    shared_ptr<MockHttpRequestDelegateProvider> _http_request_delegate_provider;
};

MATCHER_P(MatchCertificateChainToBridgeCertificate, bridge, "Certificate chain does not have bridge certificate pinned") {
    return arg.back() == bridge->GetCertificate();
}

TEST_F(TestBridgeHttpClient, ExecuteBridgeRequestWithNullBridge_ReturnsNull) {
    EXPECT_EQ(_httpClient->ExecuteHttpRequest(nullptr, "", ""), nullptr);

    EXPECT_CALL(*_http_request_delegate_provider, get_delegate(_, _, _, _, _, _))
            .Times(0);
    _httpClient->ExecuteHttpRequest(nullptr, "", "", "", {});
}

TEST_F(TestBridgeHttpClient, CreateEmptyHttpsBridge) {
    auto bridge = create_empty_https_bridge();
    EXPECT_TRUE(bridge->IsValidApiVersion());
}

TEST_F(TestBridgeHttpClient, ExecuteBridgeRequestWithEmptyBridge) {
    auto bridge = create_empty_https_bridge();
    auto method = HTTP_REQUEST_POST;
    auto url = "dummy_url";
    auto body = "dummy_body";

    auto http_request = create_mock_request(url);
    EXPECT_CALL(*http_request, do_request(method, body, _, _));

    auto requestInfo = _httpClient->ExecuteHttpRequest(bridge, method, url, body);

    ASSERT_NE(requestInfo, nullptr);
    EXPECT_EQ(requestInfo->GetMethod(), method);
    EXPECT_EQ(requestInfo->GetUrl(), url);
    EXPECT_EQ(requestInfo->GetBody(), body);

    EXPECT_FALSE(requestInfo->SslVerificationEnabled());
    EXPECT_TRUE(requestInfo->GetTrustedCertificates().empty());
    EXPECT_TRUE(requestInfo->GetExpectedCommonName().empty());
}

TEST_F(TestBridgeHttpClient, CreateBridgeRequestWithHttpsBridgeAndPinnedCertificate) {
    auto bridge = create_empty_https_bridge();
    const auto method = HTTP_REQUEST_POST;
    const auto url = "dummy_url";
    const auto body = "dummy_body";
    const auto certificate = "dummy_certificate";

    bridge->SetCertificate(certificate);

    auto http_request = create_mock_request(url);
    EXPECT_CALL(*http_request, do_request(method, body, _, _));

    auto requestInfo = _httpClient->ExecuteHttpRequest(bridge, method, url, body);
    ASSERT_NE(requestInfo, nullptr);

    EXPECT_TRUE(requestInfo->SslVerificationEnabled());

    ASSERT_FALSE(requestInfo->GetTrustedCertificates().empty());
    EXPECT_EQ(requestInfo->GetTrustedCertificates().back(), certificate);

    EXPECT_EQ(requestInfo->GetExpectedCommonName(), support::to_lower_case(bridge->GetId()));
}

TEST_F(TestBridgeHttpClient, ExecuteBridgeRequestWithHttpsBridge_ResponseHasCertificateChain_BridgeGetsCertificateStored_NextRequestHasSslVerification) {
    auto bridge = create_empty_https_bridge();
    const auto method = HTTP_REQUEST_POST;
    const auto url = "dummy_url";
    const auto body = "dummy_body";
    const auto bridge_certificate = "bridge_cert";
    HttpResponse response(0, body);
    response.set_certificate_chain({"root",
                                    "intermediate",
                                    bridge_certificate
                                   });

    EXPECT_TRUE(bridge->GetCertificate().empty());

    {
        auto http_request = create_mock_request(url);

        EXPECT_CALL(*http_request, fake_response()).WillOnce(Return(response));

        EXPECT_CALL(*http_request, set_verify_ssl(true)).Times(0);
        EXPECT_CALL(*http_request, set_verify_ssl(false)).Times(AtLeast(0));

        auto responseInfo = _httpClient->ExecuteHttpRequest(bridge, method, url, body);

        EXPECT_TRUE(responseInfo->GetSuccess());
        EXPECT_EQ(responseInfo->GetResponse(), body);
        EXPECT_EQ(bridge->GetCertificate(), bridge_certificate);
    }

    {
        auto http_request = create_mock_request(url);

        EXPECT_CALL(*http_request, set_verify_ssl(true));
        EXPECT_CALL(*http_request, expect_common_name(support::to_lower_case(bridge->GetId())));
        EXPECT_CALL(*http_request, set_trusted_certs(MatchCertificateChainToBridgeCertificate(bridge)));

        _httpClient->ExecuteHttpRequest(bridge, method, url, body);
    }
}
