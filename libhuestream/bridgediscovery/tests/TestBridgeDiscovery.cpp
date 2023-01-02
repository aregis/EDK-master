/*******************************************************************************
 Copyright (C) 2018 Philips Lighting Holding B.V.
 All Rights Reserved.
 ********************************************************************************/

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <future>
#include <memory>
#include <mutex>

#include "bridgediscovery/BridgeDiscovery.h"
#include "bridgediscovery/BridgeDiscoveryResult.h"
#include "bridgediscovery/BridgeDiscoveryConst.h"
#include "method/BridgeDiscoveryMethodFactory.h"

#include "bridgediscovery/mock/method/MockBridgeDiscoveryMethod.h"
#include "bridgediscovery/mock/method/MockBridgeDiscoveryMethodFactory.h"
#include "support/mock/network/http/MockHttpRequest.h"
#include "support/mock/network/http/MockHttpRequestDelegateProvider.h"

using std::atomic;
using std::condition_variable;
using std::mutex;
using std::shared_ptr;
using std::unique_lock;
using std::chrono::milliseconds;
using std::promise;
using std::vector;

using support::HttpRequestDelegator;

using huesdk::BridgeDiscovery;
using huesdk::IBridgeDiscoveryCallback;
using huesdk::BridgeDiscoveryCallback;
using huesdk::BridgeDiscoveryResult;
using huesdk::BRIDGE_DISCOVERY_RETURN_CODE_BUSY;
using huesdk::BRIDGE_DISCOVERY_RETURN_CODE_MISSING_DISCOVERY_METHODS;
using huesdk::BRIDGE_DISCOVERY_RETURN_CODE_SUCCESS;
using huesdk::BRIDGE_DISCOVERY_RETURN_CODE_STOPPED;

using support_unittests::MockBridgeDiscoveryMethodFactory;
using support_unittests::MockBridgeDiscoveryMethod;
using support_unittests::MockHttpRequest;
using support_unittests::MockHttpRequestDelegateProvider;

using testing::Mock;
using testing::NiceMock;
using testing::Return;
using testing::StrictMock;
using testing::Test;
using testing::UnorderedElementsAre;
using testing::AtLeast;

class TestBridgeDiscovery : public Test {
protected:
    TestBridgeDiscovery()
      : _bridge_discovery_factory(std::make_shared<StrictMock<MockBridgeDiscoveryMethodFactory>>()),
        _scoped_bridge_discovery_method_factory(
                [this](auto option) {return _bridge_discovery_factory->do_create(option);}),
        _http_request_delegate_provider(
                std::make_shared<StrictMock<MockHttpRequestDelegateProvider>>()) {}

    void SetUp() {
        HttpRequestDelegator::set_delegate_provider(_http_request_delegate_provider);
    }

    /**
     The delay determines when the callback will be called by the mocked class
     @return the predefined delay
     */
    unsigned int get_predefined_delay() {
        // Default delay is 100 ms
        return 100;
    }

    HttpRequestError get_predefined_error() {
        HttpRequestError error;
        error.set_code(HttpRequestError::ErrorCode::HTTP_REQUEST_ERROR_CODE_SUCCESS);
        return error;
    }

    /**
     The "found" results which will be provided by the callback
     @return the "found" results
     */
    vector<std::shared_ptr<BridgeDiscoveryResult>> get_predefined_results() {
        vector<std::shared_ptr<BridgeDiscoveryResult>> predefined_results;
       
        predefined_results.emplace_back(std::make_shared<huesdk::BridgeDiscoveryResult>("FAKE_MAC_1", "fake_ip_1", "", ""));
        predefined_results.emplace_back(std::make_shared<huesdk::BridgeDiscoveryResult>("FAKE_MAC_2", "fake_ip_2", "", ""));
        predefined_results.emplace_back(std::make_shared<huesdk::BridgeDiscoveryResult>("FAKE_MAC_3", "fake_ip_3", "", ""));
        predefined_results.emplace_back(std::make_shared<huesdk::BridgeDiscoveryResult>("FAKE_MAC_1", "fake_ip_1", "", ""));
        predefined_results.emplace_back(std::make_shared<huesdk::BridgeDiscoveryResult>("FAKE_MAC_2", "fake_ip_2", "", ""));
        predefined_results.emplace_back(std::make_shared<huesdk::BridgeDiscoveryResult>("FAKE_MAC_3", "fake_ip_3", "", ""));
        
        return predefined_results;
    }

    HttpResponse get_predefined_response(int index) {
        string body = "{\"name\":\"name1\",\"datastoreversion\":\"2\",\"apiversion\": \"1.16.0\",\"modelid\": \"BSB001\",\"swversion\": \"01023599\",\"mac\": \"FAKE_MAC_" + std::to_string(index) + "\", \"bridgeid\":\"FAKE_MAC_" + std::to_string(index) + "\"}";
        HttpResponse response;

        response.set_status_code(200);
        response.set_body(body);

        return response;
    }

    void setup_get_config_mocks_and_expects() {
        for (int i = 1; i < 4; i++) {
            string url_config = "http://fake_ip_" + std::to_string(i) +
                                huesdk::bridge_discovery_const::IPCHECK_BRIDGE_CONFIG_HTTP_URL_PATH;
            shared_ptr<MockHttpRequest> http_request_config = shared_ptr<MockHttpRequest>(
                    new NiceMock<MockHttpRequest>(url_config, 0));

            // Expect:
            EXPECT_CALL(*_http_request_delegate_provider, get_delegate(url_config, _, _, _, _, _))
                    .WillOnce(Return(http_request_config));

            // Expect:
            EXPECT_CALL(*http_request_config, do_get(_));
            EXPECT_CALL(*http_request_config, fake_delay())
                    .WillRepeatedly(Return(get_predefined_delay()));
            EXPECT_CALL(*http_request_config, fake_error())
                    .WillRepeatedly(Return(get_predefined_error()));
            EXPECT_CALL(*http_request_config, fake_response())
                    .WillRepeatedly(Return(get_predefined_response(i)));
        }
    }

    std::shared_ptr<MockBridgeDiscoveryMethodFactory> _bridge_discovery_factory;

    huesdk::ScopedBridgeDiscoveryMethodFactory _scoped_bridge_discovery_method_factory;

    BridgeDiscovery _bridge_discovery;
    shared_ptr<MockHttpRequestDelegateProvider> _http_request_delegate_provider;
};

TEST_F(TestBridgeDiscovery, Search_WithoutOptions_WithCallback__PerfomsUpnpAndNupnpSearch_With6Results_AndFilters3DuplicatResults__CallbackCalledWith3Bridges) {
    auto bridge_discovery_method1 = new NiceMock<MockBridgeDiscoveryMethod>();
    auto bridge_discovery_method2 = new NiceMock<MockBridgeDiscoveryMethod>();

    // Expect: get_discovery_method will be called twice for upnp & nupnp and returns the mocked discovery method
    EXPECT_CALL(*_bridge_discovery_factory, get_discovery_method_mock(BridgeDiscovery::Option::UPNP))
            .WillOnce(Return(bridge_discovery_method1));
    EXPECT_CALL(*_bridge_discovery_factory, get_discovery_method_mock(BridgeDiscovery::Option::NUPNP))
            .WillOnce(Return(bridge_discovery_method2));

    // Expect: search will be called on both discovery methods and returns the predefined results
    EXPECT_CALL(*bridge_discovery_method1, search(_));
    EXPECT_CALL(*bridge_discovery_method2, search(_));

    EXPECT_CALL(*bridge_discovery_method1, fake_delay())
        .WillRepeatedly(Return(get_predefined_delay()));
    EXPECT_CALL(*bridge_discovery_method1, fake_results())
        .WillRepeatedly(Return(get_predefined_results()));
    
    EXPECT_CALL(*bridge_discovery_method2, fake_delay())
        .WillRepeatedly(Return(get_predefined_delay()));
    EXPECT_CALL(*bridge_discovery_method2, fake_results())
        .WillRepeatedly(Return(get_predefined_results()));

    // Intialize mocked http request for retrieving config and description
    setup_get_config_mocks_and_expects();

    std::promise<void> callback_called;
    auto callback_called_future = callback_called.get_future();

    BridgeDiscoveryCallback callback([&] (const vector<std::shared_ptr<BridgeDiscoveryResult>> &results, BridgeDiscoveryReturnCode return_code) {
        // Still expect 3 results, because duplicates should be filtered
        EXPECT_EQ(3ul, results.size());
        
        EXPECT_EQ(BRIDGE_DISCOVERY_RETURN_CODE_SUCCESS, return_code);

        vector<string> result_ids(results.size());
        vector<string> result_ips(results.size());
        std::transform(results.begin(), results.end(), result_ids.begin(), [](const auto& result_entry) {
            return result_entry->get_unique_id();
        });
        std::transform(results.begin(), results.end(), result_ips.begin(), [](const auto& result_entry) {
            return result_entry->get_ip();
        });

        EXPECT_THAT(result_ids, UnorderedElementsAre("FAKE_MAC_1", "FAKE_MAC_2", "FAKE_MAC_3"));
        EXPECT_THAT(result_ips, UnorderedElementsAre("fake_ip_1", "fake_ip_2", "fake_ip_3"));

        callback_called.set_value();
    });

    // Start search
    _bridge_discovery.search(&callback);

    callback_called_future.wait();
}

TEST_F(TestBridgeDiscovery, Search_WithoutOptions_WithCallback__PerformsUpnpAndNupnpAndIpSearch_With6Results_Filters3DuplicatResults__CallbackCalledWith3Bridges) {
    auto bridge_discovery_method1 = new NiceMock<MockBridgeDiscoveryMethod>();
    auto bridge_discovery_method2 = new NiceMock<MockBridgeDiscoveryMethod>();
    auto bridge_discovery_method3 = new NiceMock<MockBridgeDiscoveryMethod>();

    // Expect: get_discovery_method will be called for all options and returns the mocked discovery method
    EXPECT_CALL(*_bridge_discovery_factory, get_discovery_method_mock(BridgeDiscovery::Option::UPNP))
            .WillOnce(Return(bridge_discovery_method1));
    EXPECT_CALL(*_bridge_discovery_factory, get_discovery_method_mock(BridgeDiscovery::Option::NUPNP))
            .WillOnce(Return(bridge_discovery_method2));
    EXPECT_CALL(*_bridge_discovery_factory, get_discovery_method_mock(BridgeDiscovery::Option::IPSCAN))
            .WillOnce(Return(bridge_discovery_method3));
    
    // Expect: search will be called on all discovery methods and returns the predefined results
    EXPECT_CALL(*bridge_discovery_method1, search(_));
    EXPECT_CALL(*bridge_discovery_method2, search(_));
    EXPECT_CALL(*bridge_discovery_method3, search(_));

    EXPECT_CALL(*bridge_discovery_method1, fake_delay())
        .WillRepeatedly(Return(get_predefined_delay()));
    EXPECT_CALL(*bridge_discovery_method1, fake_results())
        .WillRepeatedly(Return(get_predefined_results()));
    
    EXPECT_CALL(*bridge_discovery_method2, fake_delay())
        .WillRepeatedly(Return(get_predefined_delay()));
    EXPECT_CALL(*bridge_discovery_method2, fake_results())
        .WillRepeatedly(Return(get_predefined_results()));
    
    EXPECT_CALL(*bridge_discovery_method3, fake_delay())
        .WillRepeatedly(Return(get_predefined_delay()));
    EXPECT_CALL(*bridge_discovery_method3, fake_results())
        .WillRepeatedly(Return(get_predefined_results()));

    std::promise<void> callback_called;
    auto callback_called_future = callback_called.get_future();

    // Intialize mocked http request for retrieving config and description
    setup_get_config_mocks_and_expects();

    BridgeDiscoveryCallback callback = BridgeDiscoveryCallback([&] (const vector<std::shared_ptr<BridgeDiscoveryResult>> &results, BridgeDiscoveryReturnCode return_code) {
        EXPECT_EQ(3ul, results.size());
        EXPECT_EQ(BRIDGE_DISCOVERY_RETURN_CODE_SUCCESS, return_code);

        vector<string> result_ids(results.size());
        vector<string> result_ips(results.size());
        std::transform(results.begin(), results.end(), result_ids.begin(), [](const auto& result_entry) {
            return result_entry->get_unique_id();
        });
        std::transform(results.begin(), results.end(), result_ips.begin(), [](const auto& result_entry) {
            return result_entry->get_ip();
        });

        EXPECT_THAT(result_ids, testing::UnorderedElementsAre("FAKE_MAC_1", "FAKE_MAC_2", "FAKE_MAC_3"));
        EXPECT_THAT(result_ips, testing::UnorderedElementsAre("fake_ip_1", "fake_ip_2", "fake_ip_3"));

        callback_called.set_value();
    });

    // Start search
    _bridge_discovery.search(BridgeDiscovery::Option::UPNP | BridgeDiscovery::Option::NUPNP | BridgeDiscovery::Option::IPSCAN, &callback);

    callback_called_future.wait();
}

TEST_F(TestBridgeDiscovery, Search_WithAllOptions_WithCallback__PerformsUpnpAndNupnpAndIpSearch_WithNoResults__CallbackCalledWithNoBridges) {
    auto bridge_discovery_method1 = new NiceMock<MockBridgeDiscoveryMethod>();
    auto bridge_discovery_method2 = new NiceMock<MockBridgeDiscoveryMethod>();
    auto bridge_discovery_method3 = new NiceMock<MockBridgeDiscoveryMethod>();

    // Expect: get_discovery_method will be called for all options and returns the mocked discovery method
    EXPECT_CALL(*_bridge_discovery_factory, get_discovery_method_mock(BridgeDiscovery::Option::UPNP))
            .WillOnce(Return(bridge_discovery_method1));
    EXPECT_CALL(*_bridge_discovery_factory, get_discovery_method_mock(BridgeDiscovery::Option::NUPNP))
            .WillOnce(Return(bridge_discovery_method2));
    EXPECT_CALL(*_bridge_discovery_factory, get_discovery_method_mock(BridgeDiscovery::Option::IPSCAN))
            .WillOnce(Return(bridge_discovery_method3));
    
    // Expect: search will be called on all discovery methods and returns the predefined results
    EXPECT_CALL(*bridge_discovery_method1, search(_));
    EXPECT_CALL(*bridge_discovery_method2, search(_));
    EXPECT_CALL(*bridge_discovery_method3, search(_));

    EXPECT_CALL(*bridge_discovery_method1, fake_delay())
        .WillRepeatedly(Return(get_predefined_delay()));
    EXPECT_CALL(*bridge_discovery_method1, fake_results())
        .WillRepeatedly(Return(vector<std::shared_ptr<BridgeDiscoveryResult>>()));
    
    EXPECT_CALL(*bridge_discovery_method2, fake_delay())
        .WillRepeatedly(Return(get_predefined_delay()));
    EXPECT_CALL(*bridge_discovery_method2, fake_results())
        .WillRepeatedly(Return(vector<std::shared_ptr<BridgeDiscoveryResult>>()));
    
    EXPECT_CALL(*bridge_discovery_method3, fake_delay())
        .WillRepeatedly(Return(get_predefined_delay()));
    EXPECT_CALL(*bridge_discovery_method3, fake_results())
        .WillRepeatedly(Return(vector<std::shared_ptr<BridgeDiscoveryResult>>()));


    std::promise<void> callback_called;
    auto callback_called_future = callback_called.get_future();

    BridgeDiscoveryCallback callback = BridgeDiscoveryCallback([&] (vector<std::shared_ptr<BridgeDiscoveryResult>> results, BridgeDiscoveryReturnCode return_code) {
        EXPECT_EQ(0ul, results.size());
        EXPECT_EQ(BRIDGE_DISCOVERY_RETURN_CODE_SUCCESS, return_code);

        callback_called.set_value();
    });

    // Start search
    _bridge_discovery.search(BridgeDiscovery::Option::UPNP | BridgeDiscovery::Option::NUPNP | BridgeDiscovery::Option::IPSCAN, &callback);

    callback_called_future.wait();
}

TEST_F(TestBridgeDiscovery, Search_WithInvalidOptions_WithCallback__ReturnsError) {
    std::promise<BridgeDiscoveryReturnCode> callback_called;
    auto callback_called_future = callback_called.get_future();

    BridgeDiscoveryCallback callback = BridgeDiscoveryCallback(
        [&] (const vector<std::shared_ptr<BridgeDiscoveryResult>>& /*results*/, BridgeDiscoveryReturnCode return_code) {
            callback_called.set_value(return_code);
        });

    // Start search
    _bridge_discovery.search(static_cast<BridgeDiscovery::Option>(1000), &callback);
    auto result = callback_called_future.get();
    EXPECT_EQ(BRIDGE_DISCOVERY_RETURN_CODE_MISSING_DISCOVERY_METHODS, result);
}

TEST_F(TestBridgeDiscovery, Search_WithAllOptions_WithCallback__UpnpAndNupnpAndIpMethodsNotProvided__CallbackCalledWithNoBridges) {
    // Expect: get_discovery_method will be called for all options and returns a nullptr
    EXPECT_CALL(*_bridge_discovery_factory, get_discovery_method_mock(BridgeDiscovery::Option::UPNP))
            .WillOnce(Return(nullptr));
    EXPECT_CALL(*_bridge_discovery_factory, get_discovery_method_mock(BridgeDiscovery::Option::NUPNP))
            .WillOnce(Return(nullptr));
    EXPECT_CALL(*_bridge_discovery_factory, get_discovery_method_mock(BridgeDiscovery::Option::IPSCAN))
            .WillOnce(Return(nullptr));


    std::promise<void> callback_called;
    auto callback_called_future = callback_called.get_future();

    BridgeDiscoveryCallback callback = BridgeDiscoveryCallback(
            [&] (vector<std::shared_ptr<BridgeDiscoveryResult>> /*results*/, BridgeDiscoveryReturnCode return_code) {
                EXPECT_EQ(BRIDGE_DISCOVERY_RETURN_CODE_MISSING_DISCOVERY_METHODS, return_code);
                callback_called.set_value();
            });

    // Start search
    _bridge_discovery.search(BridgeDiscovery::Option::UPNP | BridgeDiscovery::Option::NUPNP | BridgeDiscovery::Option::IPSCAN, &callback);
    callback_called_future.wait();
}

TEST_F(TestBridgeDiscovery, Search_WithAllOptions_WithoutCallback__ReturnsError) {
    ASSERT_NO_FATAL_FAILURE(_bridge_discovery.search(BridgeDiscovery::Option::UPNP | BridgeDiscovery::Option::NUPNP | BridgeDiscovery::Option::IPSCAN, nullptr));
}

TEST_F(TestBridgeDiscovery, Search_WithoutOptions_WithCallback__WhileAlreadySearchInProgress_ReturnsError) {
    auto bridge_discovery_method1 = new NiceMock<MockBridgeDiscoveryMethod>();
    auto bridge_discovery_method2 = new NiceMock<MockBridgeDiscoveryMethod>();

    // Expect: get_discovery_method will be called twice for upnp & nupnp and returns the mocked discovery method
    EXPECT_CALL(*_bridge_discovery_factory, get_discovery_method_mock(BridgeDiscovery::Option::UPNP))
            .WillOnce(Return(bridge_discovery_method1));
    EXPECT_CALL(*_bridge_discovery_factory, get_discovery_method_mock(BridgeDiscovery::Option::NUPNP))
            .WillOnce(Return(bridge_discovery_method2));
    
    // Expect: search will be called on both discovery methods and returns the predefined results
    EXPECT_CALL(*bridge_discovery_method1, search(_));
    EXPECT_CALL(*bridge_discovery_method2, search(_));

    EXPECT_CALL(*bridge_discovery_method1, fake_delay())
        .WillRepeatedly(Return(get_predefined_delay()));
    EXPECT_CALL(*bridge_discovery_method1, fake_results())
        .WillRepeatedly(Return(get_predefined_results()));
    
    EXPECT_CALL(*bridge_discovery_method2, fake_delay())
        .WillRepeatedly(Return(get_predefined_delay()));
    EXPECT_CALL(*bridge_discovery_method2, fake_results())
        .WillRepeatedly(Return(get_predefined_results()));

    // Intialize mocked http request for retrieving config and description
    setup_get_config_mocks_and_expects();

    promise<void> callback_called_success;
    promise<void> callback_called_busy;

    auto callback_called_success_future = callback_called_success.get_future();
    auto callback_called_busy_future = callback_called_busy.get_future();

    BridgeDiscoveryCallback callback = BridgeDiscoveryCallback([&] (vector<std::shared_ptr<BridgeDiscoveryResult>> /*results*/, BridgeDiscoveryReturnCode return_code) {
        if (return_code == BRIDGE_DISCOVERY_RETURN_CODE_SUCCESS) {
            callback_called_success.set_value();
        }

        if (return_code == BRIDGE_DISCOVERY_RETURN_CODE_BUSY) {
            callback_called_busy.set_value();
        }
    });

    // Start search
    _bridge_discovery.search(&callback);
    // Start search again
    _bridge_discovery.search(&callback);

    callback_called_success_future.wait();
    callback_called_busy_future.wait();
}

TEST_F(TestBridgeDiscovery, Search_WithUpnpOption_WithCallback__UpnpSearchReturnsError__CallbackCalledWithNoBridges) {
    auto bridge_discovery_method1 = new NiceMock<MockBridgeDiscoveryMethod>();

    // Expect: get_discovery_method will be called once for upnp and returns the mocked discovery method
    EXPECT_CALL(*_bridge_discovery_factory, get_discovery_method_mock(BridgeDiscovery::Option::UPNP))
            .WillOnce(Return(bridge_discovery_method1));


    // Expect: search will be called on the discovery methods and returns the predefined results
    EXPECT_CALL(*bridge_discovery_method1, search(_)).WillOnce(Invoke([](shared_ptr<IBridgeDiscoveryCallback> callback){
        vector<std::shared_ptr<BridgeDiscoveryResult>> results;
        (*callback)(std::move(results), huesdk::BRIDGE_DISCOVERY_RETURN_CODE_BUSY);
    }));

    std::promise<void> callback_called;
    auto callback_called_future = callback_called.get_future();

    BridgeDiscoveryCallback callback = BridgeDiscoveryCallback([&] (vector<std::shared_ptr<BridgeDiscoveryResult>> results, BridgeDiscoveryReturnCode return_code) {
        EXPECT_EQ(0ul, results.size());
        EXPECT_EQ(BRIDGE_DISCOVERY_RETURN_CODE_SUCCESS, return_code);

        callback_called.set_value();
    });

    // Start search
    _bridge_discovery.search(BridgeDiscovery::Option::UPNP, &callback);

    callback_called_future.wait();
}

TEST_F(TestBridgeDiscovery, IsSearching__NoSearchInProgress__ReturnsFalse) {
    EXPECT_FALSE(_bridge_discovery.is_searching());
}

TEST_F(TestBridgeDiscovery, IsSearching__WhileSearchInProgress__ReturnsTrue) {
    auto bridge_discovery_method1 = new NiceMock<MockBridgeDiscoveryMethod>();

    // Expect: get_discovery_method will be called once for upnp and returns the mocked discovery method
    EXPECT_CALL(*_bridge_discovery_factory, get_discovery_method_mock(BridgeDiscovery::Option::UPNP))
            .WillOnce(Return(bridge_discovery_method1));

    // Expect: search will be called on the discovery methods and returns the predefined results
    EXPECT_CALL(*bridge_discovery_method1, search(_));
    EXPECT_CALL(*bridge_discovery_method1, fake_delay())
        .WillRepeatedly(Return(get_predefined_delay()));
    EXPECT_CALL(*bridge_discovery_method1, fake_results())
        .WillRepeatedly(Return(get_predefined_results()));

    setup_get_config_mocks_and_expects();

    std::promise<void> callback_called;
    auto callback_called_future = callback_called.get_future();

    BridgeDiscoveryCallback callback = BridgeDiscoveryCallback([&] (vector<std::shared_ptr<BridgeDiscoveryResult>> /*results*/, BridgeDiscoveryReturnCode return_code) {
        EXPECT_EQ(BRIDGE_DISCOVERY_RETURN_CODE_SUCCESS, return_code);

        callback_called.set_value();
    });

    // Start search
    _bridge_discovery.search(BridgeDiscovery::Option::UPNP, &callback);

    EXPECT_TRUE(_bridge_discovery.is_searching());

    callback_called_future.wait();
}

TEST_F(TestBridgeDiscovery, IsSearching__SearchJustFinished__ReturnsFalse) {
    auto bridge_discovery_method1 = new NiceMock<MockBridgeDiscoveryMethod>();

    // Expect: get_discovery_method will be called once for upnp and returns the mocked discovery method
    EXPECT_CALL(*_bridge_discovery_factory, get_discovery_method_mock(BridgeDiscovery::Option::IPSCAN))
            .WillOnce(Return(bridge_discovery_method1));

    // Expect: search will be called on the discovery methods and returns the predefined results
    EXPECT_CALL(*bridge_discovery_method1, search(_));
    EXPECT_CALL(*bridge_discovery_method1, fake_delay())
        .WillRepeatedly(Return(get_predefined_delay()));
    EXPECT_CALL(*bridge_discovery_method1, fake_results())
        .WillRepeatedly(Return(get_predefined_results()));

    setup_get_config_mocks_and_expects();

    std::promise<void> callback_called;
    auto callback_called_future = callback_called.get_future();

    BridgeDiscoveryCallback callback = BridgeDiscoveryCallback([&] (vector<std::shared_ptr<BridgeDiscoveryResult>> /*results*/, BridgeDiscoveryReturnCode return_code) {
        EXPECT_EQ(BRIDGE_DISCOVERY_RETURN_CODE_SUCCESS, return_code);
        callback_called.set_value();
    });

    // Start search
    _bridge_discovery.search(BridgeDiscovery::Option::IPSCAN, &callback);
    
    // Wait until callback is called
    callback_called_future.wait();

    // Check whether not searching
    EXPECT_FALSE(_bridge_discovery.is_searching());
}

TEST_F(TestBridgeDiscovery, Stop__NoSearchInProgress__CallbackCalled_ReturnsIsSearchingFalse) {
    // Check whether not searching
    EXPECT_FALSE(_bridge_discovery.is_searching());
    
    // Stop search
    _bridge_discovery.stop();
    
    EXPECT_FALSE(_bridge_discovery.is_searching());
}

TEST_F(TestBridgeDiscovery, Stop__WhileSearchInProgress_AndPerformsIpScan_WillBeStopped__CallbackCalled_ReturnsIsSearchingFalse) {
    auto bridge_discovery_method1 = new NiceMock<MockBridgeDiscoveryMethod>();

    /* expectations */

    // Expect: get_discovery_method will be called for all options and returns the mocked discovery method
    EXPECT_CALL(*_bridge_discovery_factory, get_discovery_method_mock(BridgeDiscovery::Option::IPSCAN))
            .WillOnce(Return(bridge_discovery_method1));

    // Expect: search will be called only on the first discovery method
    EXPECT_CALL(*bridge_discovery_method1, search(_));
    // Expect: stop will be called only on the first discovery method
    EXPECT_CALL(*bridge_discovery_method1, stop());
    EXPECT_CALL(*bridge_discovery_method1, fake_delay())
        .WillRepeatedly(Return(get_predefined_delay()));
    EXPECT_CALL(*bridge_discovery_method1, fake_results())
        .WillRepeatedly(Return(get_predefined_results()));
    
    promise<void> callback_called;
    auto callback_called_future = callback_called.get_future();

    BridgeDiscoveryCallback callback = BridgeDiscoveryCallback([&] (vector<std::shared_ptr<BridgeDiscoveryResult>> /*results*/, BridgeDiscoveryReturnCode return_code) {
        EXPECT_EQ(BRIDGE_DISCOVERY_RETURN_CODE_STOPPED, return_code);
        callback_called.set_value();
    });

    // Start search
    _bridge_discovery.search(BridgeDiscovery::Option::IPSCAN, &callback);
    
    // Wait a bit before stopping the search
    std::this_thread::sleep_for(milliseconds(75));
    
    // Stop searching
    _bridge_discovery.stop();

    callback_called_future.wait();

    EXPECT_FALSE(_bridge_discovery.is_searching());
}

TEST_F(TestBridgeDiscovery, Search_PerformsUpnpSearch_ObjectLeavesScope__SearchStops__CallbackCalled) {
    auto bridge_discovery_method1 = new NiceMock<MockBridgeDiscoveryMethod>();

    EXPECT_CALL(*_bridge_discovery_factory, get_discovery_method_mock(BridgeDiscovery::Option::UPNP))
            .WillOnce(Return(bridge_discovery_method1));

    EXPECT_CALL(*bridge_discovery_method1, search(_))
            .Times(AtLeast(0));

    EXPECT_CALL(*bridge_discovery_method1, fake_delay())
            .WillRepeatedly(Return(get_predefined_delay()));
    EXPECT_CALL(*bridge_discovery_method1, fake_results())
            .WillRepeatedly(Return(vector<std::shared_ptr<BridgeDiscoveryResult>>()));

    promise<void> callback_called;
    auto callback_called_future = callback_called.get_future();

    BridgeDiscoveryCallback callback = BridgeDiscoveryCallback(
            [&](vector<std::shared_ptr<BridgeDiscoveryResult>> results, BridgeDiscoveryReturnCode return_code) {
                EXPECT_TRUE(results.empty());

                EXPECT_EQ(BRIDGE_DISCOVERY_RETURN_CODE_STOPPED, return_code);
                callback_called.set_value();
            });

    {
        // Start search and let BridgeDiscovery go out of scope
        BridgeDiscovery temporary_bridge_discovery;
        temporary_bridge_discovery.search(BridgeDiscovery::Option::UPNP, &callback);
    }

    callback_called_future.wait();
}
