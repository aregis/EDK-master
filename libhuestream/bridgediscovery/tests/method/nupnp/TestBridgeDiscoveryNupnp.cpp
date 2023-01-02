/*******************************************************************************
 Copyright (C) 2018 Philips Lighting Holding B.V.
 All Rights Reserved.
 ********************************************************************************/

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <functional>
#include <future>
#include <memory>
#include <string>
#include <boost/optional.hpp>

#include "support/network/http/HttpRequest.h"
#include "support/network/http/_test/HttpRequestDelegator.h"

#include "method/nupnp/BridgeDiscoveryNupnp.h"
#include "bridgediscovery/BridgeDiscovery.h"
#include "bridgediscovery/BridgeDiscoveryConst.h"
#include "bridgediscovery/BridgeDiscoveryResult.h"

#include "support/mock/network/http/MockHttpRequest.h"
#include "support/mock/network/http/MockHttpRequestDelegateProvider.h"

using std::string;
using std::chrono::milliseconds;
using std::make_shared;
using std::vector;
using huesdk::BridgeDiscoveryCallback;
using huesdk::BridgeDiscoveryNupnp;
using huesdk::BridgeDiscoveryResult;
using huesdk::BridgeDiscoveryReturnCode;
using huesdk::BRIDGE_DISCOVERY_RETURN_CODE_BUSY;
using huesdk::BRIDGE_DISCOVERY_RETURN_CODE_SUCCESS;
using huesdk::BRIDGE_DISCOVERY_RETURN_CODE_STOPPED;
using support::HTTP_REQUEST_STATUS_FAILED;
using support::HTTP_REQUEST_STATUS_OK;
using support::HttpRequestDelegateProviderImpl;
using support::HttpRequestDelegator;
using support::HttpRequestError;
using support::HttpResponse;
using support_unittests::MockHttpRequest;
using support_unittests::MockHttpRequestDelegateProvider;
using testing::AtLeast;
using testing::NiceMock;
using testing::Return;
using testing::StrictMock;
using testing::Test;


/* fixture */

class TestBridgeDiscoveryNupnp : public Test {
protected:
    /**
    
     */
    virtual void SetUp() {
        _http_request_delegate_provider = shared_ptr<MockHttpRequestDelegateProvider>(new StrictMock<MockHttpRequestDelegateProvider>());
        // Inject the delegate provider into the http request delegator
        _http_request_delegate_provider_backup = HttpRequestDelegator::set_delegate_provider(_http_request_delegate_provider);
        
        _http_request = shared_ptr<MockHttpRequest>(new NiceMock<MockHttpRequest>("http://meethue.com/api/nupnp/test", 0));
    }

    /** 
    
     */
    virtual void TearDown() {
        // Reset delegate to it's original value
        HttpRequestDelegator::set_delegate_provider(_http_request_delegate_provider_backup);
    
        _http_request = nullptr;
    }
    
    
    /* predefined data */
     
    /**
    
     */
    unsigned int get_predefined_delay() {
        // Default delay is 100 ms
        return 100;
    }
    
    /**
    
     */
    HttpRequestError get_predefined_error() {
        HttpRequestError error;
        
        error.set_code(HttpRequestError::ErrorCode::HTTP_REQUEST_ERROR_CODE_SUCCESS);
        error.set_message("");
        
        return error;
    }
    
    /**
    
     */
    HttpResponse get_predefined_response() {
        HttpResponse response;
        
        response.set_status_code(200);
        response.set_body("["\
            "{\"id\":\"001788fffe1055ec\", \"internalipaddress\":\"192.168.2.1\", \"macaddress\":\"00:17:88:10:55:ec\"},"\
            "{\"id\":\"001788fffe1055ed\", \"internalipaddress\":\"192.168.2.2\", \"macaddress\":\"00:17:88:10:55:ed\"},"\
            "{\"id\":\"001788fffe1055ee\", \"internalipaddress\":\"192.168.2.3\", \"macaddress\":\"00:17:88:10:55:ee\"}"\
        "]");
        
        return response;
    }

    /** */
    shared_ptr<MockHttpRequest>                 _http_request;
    /** */
    shared_ptr<MockHttpRequestDelegateProvider> _http_request_delegate_provider;
    shared_ptr<HttpRequestDelegateProvider>     _http_request_delegate_provider_backup;
};


TEST_F(TestBridgeDiscoveryNupnp, Search_WithCallback__HttpRequestExecuted_AndValidJsonResponse__CallbackCalledWith3Bridges) {
    // Expect: get_delegate will be called once and the mocked http request will be returned
    EXPECT_CALL(*_http_request_delegate_provider, get_delegate(_, _, _, _, _, _))
            .WillOnce(Return(_http_request));

    // Expect:
    EXPECT_CALL(*_http_request, do_get(_));
    EXPECT_CALL(*_http_request, fake_delay())
            .WillRepeatedly(Return(get_predefined_delay()));
    EXPECT_CALL(*_http_request, fake_error())
            .WillRepeatedly(Return(get_predefined_error()));
    EXPECT_CALL(*_http_request, fake_response())
            .WillRepeatedly(Return(get_predefined_response()));

    std::promise<void> callback_called;
    vector<std::shared_ptr<BridgeDiscoveryResult>> the_results;
    boost::optional<BridgeDiscoveryReturnCode> the_return_code;

    // Start nupnp search
    BridgeDiscoveryNupnp discovery_nupnp;
    discovery_nupnp.search(make_shared<BridgeDiscoveryCallback>(
            [&](vector<std::shared_ptr<BridgeDiscoveryResult>> results, BridgeDiscoveryReturnCode return_code) {
                the_results = std::move(results);
                the_return_code = return_code;
                callback_called.set_value();
            }));

    callback_called.get_future().wait();

    ASSERT_EQ(3ul, the_results.size());
    ASSERT_EQ(BRIDGE_DISCOVERY_RETURN_CODE_SUCCESS, the_return_code.value());

    std::sort(the_results.begin(), the_results.end(), [](
            const auto &item1, const auto &item2) {
        return strcmp(item1->get_ip(), item2->get_ip()) < 0;
    });

    EXPECT_STREQ("192.168.2.1", the_results[0]->get_ip());
    EXPECT_STREQ("192.168.2.2", the_results[1]->get_ip());
    EXPECT_STREQ("192.168.2.3", the_results[2]->get_ip());
}

TEST_F(TestBridgeDiscoveryNupnp, Search_WithCallback__HttpRequestExecuted_AndEmptyJsonResponse__CallbackCalledWithNoBridges) {
    // Expect: get_delegate will be called once and the mocked http request will be returned
    EXPECT_CALL(*_http_request_delegate_provider, get_delegate(_, _, _, _, _, _))
        .WillOnce(Return(_http_request));

    // Expect:
    EXPECT_CALL(*_http_request, do_get(_));
    EXPECT_CALL(*_http_request, fake_delay())
        .WillRepeatedly(Return(get_predefined_delay()));
    EXPECT_CALL(*_http_request, fake_error())
        .WillRepeatedly(Return(get_predefined_error()));
    EXPECT_CALL(*_http_request, fake_response())
        .WillRepeatedly(Return(HttpResponse(200, "[]")));

    std::promise<void> callback_called;

    // Start nupnp search
    BridgeDiscoveryNupnp discovery_nupnp;
    discovery_nupnp.search(make_shared<BridgeDiscoveryCallback>([&] (vector<std::shared_ptr<BridgeDiscoveryResult>> results, BridgeDiscoveryReturnCode return_code) {
        EXPECT_EQ(0ul, results.size());
        EXPECT_EQ(BRIDGE_DISCOVERY_RETURN_CODE_SUCCESS, return_code);

        callback_called.set_value();
    }));
    
    callback_called.get_future().wait();
}

TEST_F(TestBridgeDiscoveryNupnp, Search_WithCallback__HttpRequestExecuted_AndInvalidJsonResponse__CallbackCalledWithNoBridges) {
    // Expect: get_delegate will be called once and the mocked http request will be returned
    EXPECT_CALL(*_http_request_delegate_provider, get_delegate(_, _, _, _, _, _))
        .WillOnce(Return(_http_request));

    HttpResponse response;
    // Set fake response data
    response.set_status_code(200);
    response.set_body("[*/invalid json */"\
        "{\"id\":\"001788fffe1055ec\", \"internalipaddress\":\"192.168.2.1\", \"macaddress\":\"00:17:88:10:55:ec\"},"\
    "]");

    // Expect:
    EXPECT_CALL(*_http_request, do_get(_));
    EXPECT_CALL(*_http_request, fake_delay())
        .WillRepeatedly(Return(get_predefined_delay()));
    EXPECT_CALL(*_http_request, fake_error())
        .WillRepeatedly(Return(get_predefined_error()));
    EXPECT_CALL(*_http_request, fake_response())
        .WillRepeatedly(Return(response));

    std::promise<void> callback_called;

    // Start nupnp search
    BridgeDiscoveryNupnp discovery_nupnp;
    discovery_nupnp.search(make_shared<BridgeDiscoveryCallback>([&] (vector<std::shared_ptr<BridgeDiscoveryResult>> results, BridgeDiscoveryReturnCode return_code) {
        EXPECT_EQ(0ul, results.size());
        EXPECT_EQ(BRIDGE_DISCOVERY_RETURN_CODE_SUCCESS, return_code);

        callback_called.set_value();
    }));

    callback_called.get_future().wait();
}

TEST_F(TestBridgeDiscoveryNupnp, Search_WithCallback__HttpRequestExecuted_AndPartialInvalidJsonResponse__CallbackCalledWith2Bridges) {
    // Expect: get_delegate will be called once and the mocked http request will be returned
    EXPECT_CALL(*_http_request_delegate_provider, get_delegate(_, _, _, _, _, _))
        .WillOnce(Return(_http_request));

    HttpResponse response;
    // Set fake response data
    response.set_status_code(200);
    response.set_body("["\
        "{\"id\":\"001788fffe1055ec\", \"internalipaddress\":\"192.168.2.1\", \"macaddress\":\"00:17:88:10:55:ec\"},"\
        "{\"id\":\"001788fffe1055ed\", \"internalipaddress\":\"192.168.2.2\", \"macaddress\":\"00:17:88:10:55:ed\"},"\
        "{id\":\"001788fffe1055ee\", \"internalipaddress\":\"192.168.2.3\", macaddress\":\"00:17:88:10:55:ee\"},"\
    "]");

    // Expect:
    EXPECT_CALL(*_http_request, do_get(_));
    EXPECT_CALL(*_http_request, fake_delay())
        .WillRepeatedly(Return(get_predefined_delay()));
    EXPECT_CALL(*_http_request, fake_error())
        .WillRepeatedly(Return(get_predefined_error()));
    EXPECT_CALL(*_http_request, fake_response())
        .WillRepeatedly(Return(response));

    std::promise<void> callback_called;

    // Start nupnp search
    BridgeDiscoveryNupnp discovery_nupnp;
    discovery_nupnp.search(make_shared<BridgeDiscoveryCallback>([&] (vector<std::shared_ptr<BridgeDiscoveryResult>> results, BridgeDiscoveryReturnCode return_code) {
        ASSERT_EQ(0ul, results.size());
        EXPECT_EQ(BRIDGE_DISCOVERY_RETURN_CODE_SUCCESS, return_code);

        callback_called.set_value();
    }));

    callback_called.get_future().wait();
}

TEST_F(TestBridgeDiscoveryNupnp, Search_WithCallback__HttpRequestExecuted_AndJsonResponseWithMissingAttribute__CallbackCalledWith2Bridges) {
    // Expect: get_delegate will be called once and the mocked http request will be returned
    EXPECT_CALL(*_http_request_delegate_provider, get_delegate(_, _, _, _, _, _))
        .WillOnce(Return(_http_request));

    HttpResponse response;
    // Set fake response data
    response.set_status_code(200);
    response.set_body("["\
        "{\"id\":\"001788fffe1055ec\", \"internalipaddress\":\"192.168.2.1\", \"macaddress\":\"00:17:88:10:55:ec\"},"\
        "{\"id\":\"001788fffe1055ed\", \"internalipaddress\":\"192.168.2.2\", \"macaddress\":\"00:17:88:10:55:ed\"},"\
        "{\"id\":\"001788fffe1055ee\", \"macaddress\":\"00:17:88:10:55:ee\"},"\
    "]");

    // Expect:
    EXPECT_CALL(*_http_request, do_get(_));
    EXPECT_CALL(*_http_request, fake_delay())
        .WillRepeatedly(Return(get_predefined_delay()));
    EXPECT_CALL(*_http_request, fake_error())
        .WillRepeatedly(Return(get_predefined_error()));
    EXPECT_CALL(*_http_request, fake_response())
        .WillRepeatedly(Return(response));

    std::promise<void> callback_called;
    vector<std::shared_ptr<BridgeDiscoveryResult>> the_results;
    boost::optional<BridgeDiscoveryReturnCode> the_return_code;

    // Start nupnp search
    BridgeDiscoveryNupnp discovery_nupnp;
    discovery_nupnp.search(make_shared<BridgeDiscoveryCallback>([&] (vector<std::shared_ptr<BridgeDiscoveryResult>> results, BridgeDiscoveryReturnCode return_code) {
        the_results = std::move(results);
        the_return_code = return_code;

        callback_called.set_value();
    }));

    callback_called.get_future().wait();

    ASSERT_EQ(2ul, the_results.size());
    ASSERT_EQ(BRIDGE_DISCOVERY_RETURN_CODE_SUCCESS, the_return_code.value());

    std::sort(the_results.begin(), the_results.end(), [](
            const auto &item1, const auto &item2) -> bool {
        return strcmp(item1->get_ip(), item2->get_ip()) < 0;
    });

    EXPECT_STREQ("192.168.2.1",      the_results[0]->get_ip());
    EXPECT_STREQ("192.168.2.2",      the_results[1]->get_ip());
}

TEST_F(TestBridgeDiscoveryNupnp, Search_WithCallback__HttpRequestFailed__CallbackCalledWithNoBridges) {
    // Expect: get_delegate will be called once and the mocked http request will be returned
    EXPECT_CALL(*_http_request_delegate_provider, get_delegate(_, _, _, _, _, _))
        .WillOnce(Return(_http_request));

    // Expect:
    EXPECT_CALL(*_http_request, do_get(_));
    EXPECT_CALL(*_http_request, fake_delay())
        .WillRepeatedly(Return(get_predefined_delay()));
    EXPECT_CALL(*_http_request, fake_error())
        .WillRepeatedly(Return(HttpRequestError("", HttpRequestError::ErrorCode::HTTP_REQUEST_ERROR_CODE_UNDEFINED)));
    EXPECT_CALL(*_http_request, fake_response())
        .WillRepeatedly(Return(HttpResponse(0, "")));

    std::promise<void> callback_called;
    
    // Start nupnp search
    BridgeDiscoveryNupnp discovery_nupnp;
    discovery_nupnp.search(make_shared<BridgeDiscoveryCallback>([&] (vector<std::shared_ptr<BridgeDiscoveryResult>> results, BridgeDiscoveryReturnCode return_code) {
        EXPECT_EQ(0ul,       results.size());
        EXPECT_EQ(BRIDGE_DISCOVERY_RETURN_CODE_SUCCESS, return_code);

        callback_called.set_value();
    }));

    callback_called.get_future().wait();
}

TEST_F(TestBridgeDiscoveryNupnp, Search_WithCallback__WhileAlreadySearchInProgress__ReturnsError) {
    // Expect: get_delegate will be called once and the mocked http request will be returned
    EXPECT_CALL(*_http_request_delegate_provider, get_delegate(_, _, _, _, _, _))
        .WillOnce(Return(_http_request));

    // Expect:
    EXPECT_CALL(*_http_request, do_get(_));
    EXPECT_CALL(*_http_request, fake_delay())
        .WillRepeatedly(Return(get_predefined_delay()));
    EXPECT_CALL(*_http_request, fake_error())
        .WillRepeatedly(Return(get_predefined_error()));
    EXPECT_CALL(*_http_request, fake_response())
        .WillRepeatedly(Return(get_predefined_response()));
    
    std::promise<void> callback_called_success;
    std::promise<void> callback_called_busy;

    auto callback = make_shared<BridgeDiscoveryCallback>([&] (vector<std::shared_ptr<BridgeDiscoveryResult>> /*results*/, BridgeDiscoveryReturnCode return_code) {
        if (return_code == BRIDGE_DISCOVERY_RETURN_CODE_SUCCESS) {
            callback_called_success.set_value();
        }

        if (return_code == BRIDGE_DISCOVERY_RETURN_CODE_BUSY) {
            callback_called_busy.set_value();
        }
    });

    // Start nupnp search
    BridgeDiscoveryNupnp discovery_nupnp;
    discovery_nupnp.search(callback);
    // Start nupnp search again
    discovery_nupnp.search(callback);

    callback_called_success.get_future().wait();
    callback_called_busy.get_future().wait();
}

TEST_F(TestBridgeDiscoveryNupnp, Search_WithoutCallback__ReturnsError) {
    BridgeDiscoveryNupnp discovery_nupnp;
    discovery_nupnp.search(nullptr);
}

TEST_F(TestBridgeDiscoveryNupnp, IsSearching__NoSearchInProgress__ReturnsFalse) {
    BridgeDiscoveryNupnp discovery_nupnp;

    EXPECT_FALSE(discovery_nupnp.is_searching());
}

TEST_F(TestBridgeDiscoveryNupnp, IsSearching__WhileSearchInProgress__ReturnsTrue) {
    // Expect: get_delegate will be called once and the mocked http request will be returned
    EXPECT_CALL(*_http_request_delegate_provider, get_delegate(_, _, _, _, _, _))
        .WillOnce(Return(_http_request));

    // Expect:
    EXPECT_CALL(*_http_request, do_get(_));
    EXPECT_CALL(*_http_request, fake_delay())
        .WillRepeatedly(Return(get_predefined_delay()));
    EXPECT_CALL(*_http_request, fake_error())
        .WillRepeatedly(Return(get_predefined_error()));
    EXPECT_CALL(*_http_request, fake_response())
        .WillRepeatedly(Return(get_predefined_response()));
    
    std::promise<void> callback_called;
    
    // Start nupnp search
    BridgeDiscoveryNupnp discovery_nupnp;
    discovery_nupnp.search(make_shared<BridgeDiscoveryCallback>([&] (vector<std::shared_ptr<BridgeDiscoveryResult>> /*results*/, BridgeDiscoveryReturnCode return_code) {
        EXPECT_EQ(BRIDGE_DISCOVERY_RETURN_CODE_SUCCESS, return_code);

        callback_called.set_value();
    }));
    // Check whether searching
    EXPECT_TRUE(discovery_nupnp.is_searching());

    callback_called.get_future().wait();
}

TEST_F(TestBridgeDiscoveryNupnp, Stop__NoSearchInProgress__IsSearchingReturnsFalse) {
    BridgeDiscoveryNupnp discovery_nupnp;

    // Check whether not searching
    EXPECT_FALSE(discovery_nupnp.is_searching());
    
    // Stop nupnp search
    discovery_nupnp.stop();
    
    EXPECT_FALSE(discovery_nupnp.is_searching());
}

TEST_F(TestBridgeDiscoveryNupnp, Stop__WhileSearchInProgress_AndHandlingHttpRequest_WhichWillBeCancelled__CallbackCalled_IsSearchingReturnsFalse) {
    // Expect: get_delegate will be called once and the mocked http request will be returned
    EXPECT_CALL(*_http_request_delegate_provider, get_delegate(_, _, _, _, _, _))
        .WillOnce(Return(_http_request));

    // Expect:
    EXPECT_CALL(*_http_request, do_get(_));
     // Expect:
    EXPECT_CALL(*_http_request, cancel());
    
    EXPECT_CALL(*_http_request, fake_delay())
        .WillRepeatedly(Return(get_predefined_delay()));
    EXPECT_CALL(*_http_request, fake_error())
        .WillRepeatedly(Return(get_predefined_error()));
    EXPECT_CALL(*_http_request, fake_response())
        .WillRepeatedly(Return(get_predefined_response()));

    std::promise<void> callback_called;

    // Start nupnp search
    BridgeDiscoveryNupnp discovery_nupnp;
    discovery_nupnp.search(make_shared<BridgeDiscoveryCallback>([&] (vector<std::shared_ptr<BridgeDiscoveryResult>> /*results*/, BridgeDiscoveryReturnCode return_code) {
        EXPECT_EQ(BRIDGE_DISCOVERY_RETURN_CODE_STOPPED, return_code);

        callback_called.set_value();
    }));
    
    // Wait a bit before stopping the search
    std::this_thread::sleep_for(milliseconds(75));
    
    // Stop nupnp search 
    discovery_nupnp.stop();

    // Wait a bit to ensure the callback is called
    std::this_thread::sleep_for(milliseconds(1000));

    callback_called.get_future().wait();

    EXPECT_FALSE(discovery_nupnp.is_searching());
}
