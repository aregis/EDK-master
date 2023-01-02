/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#ifndef LIBHUESTREAM_MOCKBRIDGEHTTPCLIENT_H
#define LIBHUESTREAM_MOCKBRIDGEHTTPCLIENT_H

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <memory>

#include "huestream/common/http/IBridgeHttpClient.h"

using testing::_;

namespace huestream {
    class MockBridgeHttpClient : public IBridgeHttpClient {
    public:
        MOCK_METHOD5(ExecuteHttpRequest, HttpRequestPtr(BridgePtr bridge, const std::string& method, const std::string& url, const std::string& body, bool supportEvent));
        MOCK_METHOD6(ExecuteHttpRequest, int32_t(BridgePtr bridge, const std::string& method, const std::string& url, const std::string& body, HttpRequestCallback callback, bool supportEvent));
				MOCK_METHOD1(CancelHttpRequest, void(int32_t requestId));
    };

    class MockWrapperBridgeHttpClient : public IBridgeHttpClient {
    public:
        explicit MockWrapperBridgeHttpClient(const std::shared_ptr<MockBridgeHttpClient>& mock)
          : _mock(mock) {}

        HttpRequestPtr ExecuteHttpRequest(BridgePtr bridge, const std::string& method, const std::string& url, const std::string& body = {}, bool supportEvent = false) override {
            return _mock->ExecuteHttpRequest(bridge, method, url, body, supportEvent);
        }

        int32_t ExecuteHttpRequest(BridgePtr bridge, const std::string& method, const std::string& url, const std::string& body, HttpRequestCallback callback, bool supportEvent = false) override {
            return _mock->ExecuteHttpRequest(bridge, method, url, body, callback, supportEvent);
        }

				void CancelHttpRequest(int32_t requestId) override {
					_mock->CancelHttpRequest(requestId);
				}

    private:
        std::shared_ptr<MockBridgeHttpClient> _mock;
    };
}

#endif //LIBHUESTREAM_MOCKBRIDGEHTTPCLIENT_H
