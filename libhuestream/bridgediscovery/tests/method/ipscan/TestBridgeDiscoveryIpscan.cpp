/*******************************************************************************
 Copyright (C) 2018 Philips Lighting Holding B.V.
 All Rights Reserved.
 ********************************************************************************/

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <future>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "support/network/_test/NetworkDelegator.h"
#include "support/network/http/HttpRequest.h"
#include "support/network/http/_test/HttpRequestDelegator.h"
#include "support/network/NetworkInterface.h"
#include "support/util/StringUtil.h"

#include "bridgediscovery/BridgeDiscovery.h"
#include "bridgediscovery/BridgeDiscoveryConst.h"
#include "bridgediscovery/BridgeDiscoveryResult.h"
#include "method/ipscan/BridgeDiscoveryIpscan.h"
#include "method/ipscan/BridgeDiscoveryIpscanPreCheck.h"

#include "support/mock/network/http/MockHttpRequest.h"
#include "support/mock/network/http/MockHttpRequestDelegateProvider.h"
#include "support/mock/network/MockNetworkDelegate.h"
#include "support/mock/network/sockets/bsd/MockCMethodDelegate.h"

using std::atomic;
using std::condition_variable;
using std::mutex;
using std::shared_ptr;
using std::make_shared;
using std::string;
using std::vector;
using std::unique_lock;
using std::chrono::milliseconds;
using huesdk::BridgeDiscoveryCallback;
using huesdk::BridgeDiscoveryIpscan;
using huesdk::BridgeDiscoveryIpscanPreCheck;
using huesdk::BridgeDiscoveryResult;
using huesdk::BridgeDiscoveryReturnCode;

using huesdk::BRIDGE_DISCOVERY_RETURN_CODE_BUSY;
using huesdk::BRIDGE_DISCOVERY_RETURN_CODE_MISSING_DISCOVERY_METHODS;
using huesdk::BRIDGE_DISCOVERY_RETURN_CODE_SUCCESS;
using huesdk::BRIDGE_DISCOVERY_RETURN_CODE_STOPPED;
using support::HTTP_REQUEST_STATUS_CANCELED;
using support::HTTP_REQUEST_STATUS_FAILED;
using support::HTTP_REQUEST_STATUS_OK;
using support::HttpRequest;
using support::HttpRequestDelegateProviderImpl;
using support::HttpRequestDelegator;
using support::HttpRequestError;
using support::HttpResponse;
using support::NetworkDelegateImpl;
using support::NetworkDelegator;
using support::INET_IPV4;
using support::INET_IPV6;
using support::NetworkInterface;
using support::to_string;
using support::CMethodDelegateDefault;
using support::CMethodDelegator;
using support_unittests::MockHttpRequest;
using support_unittests::MockHttpRequestDelegateProvider;
using support_unittests::MockNetworkDelegate;
using support_unittests::MockCMethodDelegate;
using testing::AtLeast;
using testing::NiceMock;
using testing::Return;
using testing::StrictMock;
using testing::Test;
using testing::ElementsAre;

class BridgeDiscoveryIpscanPreCheckNoop : public BridgeDiscoveryIpscanPreCheck {
    std::vector<std::string> filter_reachable_ips(std::vector<std::string> ips, const std::atomic<bool>& /*_stopped_by_user*/) override {
        return ips;
    }
};

class TestBridgeDiscoveryIpscan : public Test {
protected:
    /**
    
     */
    virtual void SetUp() {
        _cmethod_delegate = shared_ptr<MockCMethodDelegate>(new NiceMock<MockCMethodDelegate>());

        CMethodDelegator::set_delegate(_cmethod_delegate);

        _network_delegate = shared_ptr<MockNetworkDelegate>(new StrictMock<MockNetworkDelegate>());
        // Inject the network delegate into the network delegator
        NetworkDelegator::set_delegate(_network_delegate);

        _http_request_delegate_provider = shared_ptr<MockHttpRequestDelegateProvider>(new StrictMock<MockHttpRequestDelegateProvider>());
        // Inject the http request delegate provider into the http request delegator
        _http_request_delegate_provider_backup = HttpRequestDelegator::set_delegate_provider(_http_request_delegate_provider);

        BridgeDiscoveryIpscanPreCheck::set_instance(shared_ptr<BridgeDiscoveryIpscanPreCheck>(new BridgeDiscoveryIpscanPreCheckNoop()));
    }

    virtual void TearDown() {
        // Reset network delegate to it's original value
        NetworkDelegator::set_delegate(shared_ptr<NetworkDelegate>(new NetworkDelegateImpl()));
        // Reset http request delegate provider to it's original value
        HttpRequestDelegator::set_delegate_provider(_http_request_delegate_provider_backup);

        CMethodDelegator::set_delegate(shared_ptr<CMethodDelegate>(new CMethodDelegateDefault()));

        BridgeDiscoveryIpscanPreCheck::set_instance(shared_ptr<BridgeDiscoveryIpscanPreCheck>(new BridgeDiscoveryIpscanPreCheckNoop()));
    }

    const std::vector<NetworkInterface> get_predefined_network_interfaces() {
        std::vector<NetworkInterface> network_interfaces;
    
        NetworkInterface network_interface;
        network_interface.set_inet_type(INET_IPV4);
        network_interface.set_ip("192.168.1.1");
        network_interface.set_up(true);
        network_interface.set_loopback(false);
        network_interface.set_name("en0");

        NetworkInterface network_interface2;
        network_interface2.set_inet_type(INET_IPV4);
        network_interface2.set_ip("10.0.0.1");
        network_interface2.set_up(true);
        network_interface2.set_loopback(false);
        network_interface.set_name("en1");
        
        network_interfaces.push_back(network_interface);
        network_interfaces.push_back(network_interface2);
        
        return network_interfaces;
    }

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
        
        return error;
    }
    
    /**
    
     */
    HttpResponse get_predefined_response_with_mac_only() {
        HttpResponse response;
        
        response.set_status_code(200);
        response.set_body("{\"name\":\"name1\",\"datastoreversion\":\"2\",\"apiversion\": \"1.16.0\",\"modelid\": \"BSB001\",\"swversion\": \"01023599\",\"mac\": \"00:17:88:0a:e8:48\"}");
        
        return response;
    }

    HttpResponse get_predefined_response_with_no_mac_and_unique_bridge_id() {
        HttpResponse response;
        
        response.set_status_code(200);
        response.set_body("{\"name\":\"name1\",\"datastoreversion\":\"2\",\"apiversion\": \"1.16.0\",\"modelid\": \"BSB001\",\"swversion\": \"01023599\"}");
        
        return response;
    }
    
    HttpResponse get_predefined_response_with_mac_and_unique_bridge_id() {
        HttpResponse response;
        
        response.set_status_code(200);
        response.set_body("{\"name\":\"name1\",\"datastoreversion\":\"2\",\"apiversion\": \"1.16.0\",\"modelid\": \"BSB001\",\"swversion\": \"01023599\",\"mac\": \"00:17:88:0a:e8:48\", \"bridgeid\":\"001788FFFE0AE849\"}");
        
        return response;
    }
    
    HttpResponse get_predefined_response_with_description() {
        HttpResponse response;
        
        response.set_status_code(200);
        response.set_body("<modelDescription>Philips hue Personal Wireless Lighting</modelDescription>\n"\
                          "<modelName>Philips hue bridge.+</modelName>\n"\
                          "<modelNumber>929000226503</modelNumber>\n"\
                          "<modelURL>http://www.meethue.com</modelURL>\n"\
                          "<serialNumber>0017880ae848</serialNumber>");
        
        return response;
    }
    
    /** */
    BridgeDiscoveryIpscan _discovery_ipscan;
    /** */
    shared_ptr<MockNetworkDelegate>             _network_delegate;
    /** */
    shared_ptr<MockHttpRequestDelegateProvider> _http_request_delegate_provider;
    shared_ptr<HttpRequestDelegateProvider>     _http_request_delegate_provider_backup;

    shared_ptr<MockCMethodDelegate> _cmethod_delegate;
};


TEST_F(TestBridgeDiscoveryIpscan, Search_WithCallback__NetworkInterfaceAvailable_AndHttpRequestsExecuted_WithValidResponsesOnAllIps_WithEmptyConfigAndDescriptionResponse__CallbackCalledWith0Bridges) {
    // Expect:
    EXPECT_CALL(*_network_delegate, get_network_interfaces())
        .WillOnce(Return(get_predefined_network_interfaces()));

    for (unsigned int i = 1; i <= 254; i++) {
        string url_config = "http://192.168.1." + to_string(i) + huesdk::bridge_discovery_const::IPCHECK_BRIDGE_CONFIG_HTTP_URL_PATH;
//        string url_description = "http://192.168.1." + to_string(i) + "/description.xml";
        
        // Intialize mocked http request for retrieving config and description
        shared_ptr<MockHttpRequest> http_request_config      = shared_ptr<MockHttpRequest>(new NiceMock<MockHttpRequest>(url_config, 0));
//        shared_ptr<MockHttpRequest> http_request_description = shared_ptr<MockHttpRequest>(new NiceMock<MockHttpRequest>(url_description, 0));

        // Expect:
        EXPECT_CALL(*_http_request_delegate_provider, get_delegate(url_config, _, _, _, _, _))
            .WillOnce(Return(http_request_config));

//        EXPECT_CALL(*_http_request_delegate_provider, get_delegate(url_description, _, _, _, _, _))
//            .WillOnce(Return(http_request_description));
        
        // Expect:
        EXPECT_CALL(*http_request_config, do_get(_));
        EXPECT_CALL(*http_request_config, fake_delay())
            .WillRepeatedly(Return(get_predefined_delay()));
        EXPECT_CALL(*http_request_config, fake_error())
            .WillRepeatedly(Return(get_predefined_error()));
        EXPECT_CALL(*http_request_config, fake_response())
            .WillRepeatedly(Return(get_predefined_response_with_no_mac_and_unique_bridge_id()));
        
//        EXPECT_CALL(*http_request_description, do_get(_));
//        EXPECT_CALL(*http_request_description, fake_delay())
//            .WillRepeatedly(Return(get_predefined_delay()));
//        EXPECT_CALL(*http_request_description, fake_error())
//            .WillRepeatedly(Return(get_predefined_error()));
//        EXPECT_CALL(*http_request_description, fake_response())
//            .WillRepeatedly(Return(get_predefined_response_with_description()));
    }

    std::promise<void> callback_called;
    auto callback_called_future = callback_called.get_future();

    // Start ipscan
    _discovery_ipscan.search(make_shared<BridgeDiscoveryCallback>([&] (vector<std::shared_ptr<BridgeDiscoveryResult>> results, BridgeDiscoveryReturnCode return_code) {
        EXPECT_EQ(0ul, results.size());
        EXPECT_EQ(BRIDGE_DISCOVERY_RETURN_CODE_SUCCESS, return_code);

        callback_called.set_value();
    }));

    callback_called_future.wait();
}

TEST_F(TestBridgeDiscoveryIpscan, Search_WithCallback__NetworkInterfaceAvailable_AndHttpRequestsExecuted_WithValidResponsesOnAllIps_WithMacResponse__CallbackCalledWith254Bridges) {
    // Expect:
    EXPECT_CALL(*_network_delegate, get_network_interfaces())
    .WillOnce(Return(get_predefined_network_interfaces()));
    
    for (unsigned int i = 1; i <= 254; i++) {
        string url = "http://192.168.1." + to_string(i) + huesdk::bridge_discovery_const::IPCHECK_BRIDGE_CONFIG_HTTP_URL_PATH;
        
        shared_ptr<MockHttpRequest> http_request = shared_ptr<MockHttpRequest>(new NiceMock<MockHttpRequest>(url, 0));
        
        // Expect:
        EXPECT_CALL(*_http_request_delegate_provider, get_delegate(url, _, _, _, _, _))
            .WillOnce(Return(http_request));
        
        // Expect:
        EXPECT_CALL(*http_request, do_get(_));
        EXPECT_CALL(*http_request, fake_delay())
            .WillRepeatedly(Return(get_predefined_delay()));
        EXPECT_CALL(*http_request, fake_error())
            .WillRepeatedly(Return(get_predefined_error()));
        EXPECT_CALL(*http_request, fake_response())
            .WillRepeatedly(Return(get_predefined_response_with_mac_only()));
    }

    std::promise<void> callback_called;
    auto callback_called_future = callback_called.get_future();

    // Start ipscan
    _discovery_ipscan.search(make_shared<BridgeDiscoveryCallback>([&] (vector<std::shared_ptr<BridgeDiscoveryResult>> results, BridgeDiscoveryReturnCode return_code) {
        ASSERT_EQ(254ul, results.size());
        EXPECT_EQ(BRIDGE_DISCOVERY_RETURN_CODE_SUCCESS, return_code);

        // Sort the result
        std::sort(results.begin(), results.end(), [](const auto& a, const auto& b) {
            auto first_ip = string(a->get_ip());
            auto second_ip = string(b->get_ip());
            return std::stoi(first_ip.substr(first_ip.rfind('.') + 1)) < std::stoi(second_ip.substr(second_ip.rfind('.') + 1));
        });

        for (unsigned int i = 1; i <= 254; i++) {
            string ip = "192.168.1." + to_string(i);

            EXPECT_STREQ("001788FFFE0AE848", results[i-1]->get_unique_id());
            EXPECT_STREQ(ip.c_str(),         results[i-1]->get_ip());
            // NOTE: current discovery implementation: when config is without unique id, api-version is not parsed
            EXPECT_STREQ("1.16.0",           results[i-1]->get_api_version());
            EXPECT_STREQ("BSB001",           results[i-1]->get_model_id());
        }

        callback_called.set_value();
    }));
    
    callback_called_future.wait();
}

TEST_F(TestBridgeDiscoveryIpscan, Search_WithCallback__NetworkInterfaceAvailable_AndHttpRequestsExecuted_WithValidResponsesOnAllIps_WithUniqueBridgeIdResponse__CallbackCalledWith254Bridges) {
    // Expect:
    EXPECT_CALL(*_network_delegate, get_network_interfaces())
    .WillOnce(Return(get_predefined_network_interfaces()));
    
    for (unsigned int i = 1; i <= 254; i++) {
        string url = "http://192.168.1." + to_string(i) + huesdk::bridge_discovery_const::IPCHECK_BRIDGE_CONFIG_HTTP_URL_PATH;
        
        shared_ptr<MockHttpRequest> http_request = shared_ptr<MockHttpRequest>(new NiceMock<MockHttpRequest>(url, 0));
        
        // Expect:
        EXPECT_CALL(*_http_request_delegate_provider, get_delegate(url, _, _, _, _, _))
            .WillOnce(Return(http_request));
        
        // Expect:
        EXPECT_CALL(*http_request, do_get(_));
        EXPECT_CALL(*http_request, fake_delay())
            .WillRepeatedly(Return(get_predefined_delay()));
        EXPECT_CALL(*http_request, fake_error())
            .WillRepeatedly(Return(get_predefined_error()));
        EXPECT_CALL(*http_request, fake_response())
            .WillRepeatedly(Return(get_predefined_response_with_mac_and_unique_bridge_id()));
    }

    std::promise<void> callback_called;
    auto callback_called_future = callback_called.get_future();

    // Start ipscan
    _discovery_ipscan.search(make_shared<BridgeDiscoveryCallback>([&] (vector<std::shared_ptr<BridgeDiscoveryResult>> results, BridgeDiscoveryReturnCode return_code) {
        ASSERT_EQ(254ul, results.size());
        EXPECT_EQ(BRIDGE_DISCOVERY_RETURN_CODE_SUCCESS, return_code);

        // Sort the result
        std::sort(results.begin(), results.end(), [](const auto& a, const auto& b) {
            auto first_ip = string(a->get_ip());
            auto second_ip = string(b->get_ip());
            return std::stoi(first_ip.substr(first_ip.rfind('.') + 1)) < std::stoi(second_ip.substr(second_ip.rfind('.') + 1));
        });
        
        for (unsigned int i = 1; i <= 254; i++) {
            string ip = "192.168.1." + to_string(i);
            
            EXPECT_STREQ("001788FFFE0AE849", results[i-1]->get_unique_id());
            EXPECT_STREQ(ip.c_str(),         results[i-1]->get_ip());
            EXPECT_STREQ("1.16.0",      results[i-1]->get_api_version());
            EXPECT_STREQ("BSB001",      results[i-1]->get_model_id());
        }

        callback_called.set_value();
    }));

    callback_called_future.wait();
}

TEST_F(TestBridgeDiscoveryIpscan, Search_WithCallback__NetworkInterfaceAvailable_AndHttpRequestsFailedOnAllIps__CallbackCalledWithNoBridges) {
    // Expect:
    EXPECT_CALL(*_network_delegate, get_network_interfaces())
        .WillOnce(Return(get_predefined_network_interfaces()));

    for (unsigned int i = 1; i <= 254; i++) {
        string url = "http://192.168.1." + to_string(i) + huesdk::bridge_discovery_const::IPCHECK_BRIDGE_CONFIG_HTTP_URL_PATH;
        
        shared_ptr<MockHttpRequest> http_request = shared_ptr<MockHttpRequest>(new NiceMock<MockHttpRequest>(url, 0));
        
        // Expect:
        EXPECT_CALL(*_http_request_delegate_provider, get_delegate(url, _, _, _, _, _))
            .WillOnce(Return(http_request));
        
        // Expect:
        EXPECT_CALL(*http_request, do_get(_));
        EXPECT_CALL(*http_request, fake_delay())
            .WillRepeatedly(Return(get_predefined_delay()));
        EXPECT_CALL(*http_request, fake_error())
            .WillRepeatedly(Return(HttpRequestError("", HttpRequestError::ErrorCode::HTTP_REQUEST_ERROR_CODE_UNDEFINED)));
        EXPECT_CALL(*http_request, fake_response())
            .WillRepeatedly(Return(HttpResponse(0, "")));
    }

    std::promise<void> callback_called;
    auto callback_called_future = callback_called.get_future();

    // Start ipscan
    _discovery_ipscan.search(make_shared<BridgeDiscoveryCallback>([&] (vector<std::shared_ptr<BridgeDiscoveryResult>> results, BridgeDiscoveryReturnCode return_code) {
        EXPECT_EQ(0ul,       results.size());
        EXPECT_EQ(BRIDGE_DISCOVERY_RETURN_CODE_SUCCESS, return_code);

        callback_called.set_value();
    }));

    callback_called_future.wait();
}

TEST_F(TestBridgeDiscoveryIpscan, Search_WithCallback__NetworkInterfaceAvailable_AndHttpRequestsFailedOnAllIps_ExceptForValidResponseOn1Ip__CallbackCalledWith1Bridge) {
    // Expect:
    EXPECT_CALL(*_network_delegate, get_network_interfaces())
        .WillOnce(Return(get_predefined_network_interfaces()));

    // Http requests with no results
    for (unsigned int i = 1; i <= 253; i++) {
        string url = "http://192.168.1." + to_string(i) + huesdk::bridge_discovery_const::IPCHECK_BRIDGE_CONFIG_HTTP_URL_PATH;
        
        shared_ptr<MockHttpRequest> http_request = shared_ptr<MockHttpRequest>(new NiceMock<MockHttpRequest>(url, 0));

        // Expect:
        EXPECT_CALL(*_http_request_delegate_provider, get_delegate(url, _, _, _, _, _))
            .WillOnce(Return(http_request));

        // Expect:
        EXPECT_CALL(*http_request, do_get(_));
        EXPECT_CALL(*http_request, fake_delay())
            .WillRepeatedly(Return(get_predefined_delay()));
        EXPECT_CALL(*http_request, fake_error())
            .WillRepeatedly(Return(HttpRequestError("", HttpRequestError::ErrorCode::HTTP_REQUEST_ERROR_CODE_UNDEFINED)));
        EXPECT_CALL(*http_request, fake_response())
            .WillRepeatedly(Return(HttpResponse(0, "")));
    }
    
    // Intialize mocked http request for retrieving the config
    shared_ptr<MockHttpRequest> http_request_config = shared_ptr<MockHttpRequest>(new NiceMock<MockHttpRequest>("http://192.168.1.1" + huesdk::bridge_discovery_const::IPCHECK_BRIDGE_CONFIG_HTTP_URL_PATH, 0));
    
    // Expect:
    EXPECT_CALL(*_http_request_delegate_provider, get_delegate("http://192.168.1.254" + huesdk::bridge_discovery_const::IPCHECK_BRIDGE_CONFIG_HTTP_URL_PATH, _, _, _, _, _))
        .WillOnce(Return(http_request_config));
    
    // Expect:
    EXPECT_CALL(*http_request_config, do_get(_));
    EXPECT_CALL(*http_request_config, fake_delay())
        .WillRepeatedly(Return(get_predefined_delay()));
    EXPECT_CALL(*http_request_config, fake_error())
        .WillRepeatedly(Return(get_predefined_error()));
    EXPECT_CALL(*http_request_config, fake_response())
        .WillRepeatedly(Return(get_predefined_response_with_mac_only()));

    std::promise<void> callback_called;
    auto callback_called_future = callback_called.get_future();

    // Start ipscan
    _discovery_ipscan.search(make_shared<BridgeDiscoveryCallback>([&] (vector<std::shared_ptr<BridgeDiscoveryResult>> results, BridgeDiscoveryReturnCode return_code) {
        ASSERT_EQ(1ul, results.size());
        EXPECT_EQ(BRIDGE_DISCOVERY_RETURN_CODE_SUCCESS, return_code);
        
        EXPECT_STREQ("001788FFFE0AE848", results[0]->get_unique_id());
        EXPECT_STREQ("192.168.1.254",    results[0]->get_ip());
        EXPECT_STREQ("1.16.0",           results[0]->get_api_version());
        EXPECT_STREQ("BSB001",           results[0]->get_model_id());

        callback_called.set_value();
    }));

    callback_called_future.wait();
}

TEST_F(TestBridgeDiscoveryIpscan, Search_WithCallback__NoNetworkInterfaceAvailable__CallbackCalledWithNoBridges) {
    // Expect:
    EXPECT_CALL(*_network_delegate, get_network_interfaces())
        .WillOnce(Return(std::vector<NetworkInterface>()));

    std::promise<void> callback_called;
    auto callback_called_future = callback_called.get_future();

    // Start ipscan
    _discovery_ipscan.search(make_shared<BridgeDiscoveryCallback>([&] (vector<std::shared_ptr<BridgeDiscoveryResult>> results, BridgeDiscoveryReturnCode return_code) {
        EXPECT_EQ(0ul,       results.size());
        EXPECT_EQ(BRIDGE_DISCOVERY_RETURN_CODE_SUCCESS, return_code);

        callback_called.set_value();
    }));

    callback_called_future.wait();
}

TEST_F(TestBridgeDiscoveryIpscan, Search_WithCallback__NoEnabledIpv4NetworkInterfacesAvailable__CallbackCalledWithNoBridges) {
    std::vector<NetworkInterface> network_interfaces;

    NetworkInterface network_interface;
    network_interface.set_inet_type(INET_IPV4);
    network_interface.set_ip("127.0.0.1");
    network_interface.set_up(false);
    network_interface.set_loopback(true);

    NetworkInterface network_interface2;
    network_interface2.set_inet_type(INET_IPV4);
    network_interface2.set_ip("10.0.0.1");
    network_interface2.set_up(false);
    network_interface2.set_loopback(false);

    NetworkInterface network_interface3;
    network_interface3.set_inet_type(INET_IPV6);
    network_interface3.set_ip("fe80::200:f8ff:fe21:67cf");
    network_interface3.set_up(true);
    network_interface3.set_loopback(false);
    
    network_interfaces.push_back(network_interface);
    network_interfaces.push_back(network_interface2);
    network_interfaces.push_back(network_interface3);
    
    // Expect:
    EXPECT_CALL(*_network_delegate, get_network_interfaces())
        .WillOnce(Return(network_interfaces));

    std::promise<void> callback_called;
    auto callback_called_future = callback_called.get_future();

    // Start ipscan
    _discovery_ipscan.search(make_shared<BridgeDiscoveryCallback>([&] (vector<std::shared_ptr<BridgeDiscoveryResult>> results, BridgeDiscoveryReturnCode return_code) {
        EXPECT_EQ(0ul,       results.size());
        EXPECT_EQ(BRIDGE_DISCOVERY_RETURN_CODE_SUCCESS, return_code);

        callback_called.set_value();
    }));

    callback_called_future.wait();
}

TEST_F(TestBridgeDiscoveryIpscan, Search_WithCallback__NetworkInterfaceAvailable_WithInvalidIp__CallbackCalledWithNoBridges) {
    std::vector<NetworkInterface> network_interfaces;

    NetworkInterface network_interface;
    network_interface.set_inet_type(INET_IPV4);
    network_interface.set_ip("");
    network_interface.set_up(true);
    network_interface.set_loopback(false);
    
    network_interfaces.push_back(network_interface);
    
    // Expect:
    EXPECT_CALL(*_network_delegate, get_network_interfaces())
        .WillOnce(Return(network_interfaces));

    std::promise<void> callback_called;
    auto callback_called_future = callback_called.get_future();

    // Start ipscan
    _discovery_ipscan.search(make_shared<BridgeDiscoveryCallback>([&] (vector<std::shared_ptr<BridgeDiscoveryResult>> results, BridgeDiscoveryReturnCode return_code) {
        EXPECT_EQ(0ul,       results.size());
        EXPECT_EQ(BRIDGE_DISCOVERY_RETURN_CODE_SUCCESS, return_code);

        callback_called.set_value();
    }));

    callback_called_future.wait();
}

TEST_F(TestBridgeDiscoveryIpscan, Search_WithoutCallback__DoesNotCrash) {
    _discovery_ipscan.search(nullptr);
}

TEST_F(TestBridgeDiscoveryIpscan, Search_WithCallback__WhileAlreadySearchInProgress__ReturnsError) {
    // Expect:
    EXPECT_CALL(*_network_delegate, get_network_interfaces())
        .WillOnce(Return(get_predefined_network_interfaces()));

    for (unsigned int i = 1; i <= 254; i++) {
        string url = "http://192.168.1." + to_string(i) + huesdk::bridge_discovery_const::IPCHECK_BRIDGE_CONFIG_HTTP_URL_PATH;
        
        shared_ptr<MockHttpRequest> http_request = shared_ptr<MockHttpRequest>(new NiceMock<MockHttpRequest>(url, 0));
        
        // Expect:
        EXPECT_CALL(*_http_request_delegate_provider, get_delegate(url, _, _, _, _, _))
            .WillOnce(Return(http_request));
        
        // Expect:
        EXPECT_CALL(*http_request, do_get(_));
        EXPECT_CALL(*http_request, fake_delay())
            .WillRepeatedly(Return(get_predefined_delay()));
        EXPECT_CALL(*http_request, fake_error())
            .WillRepeatedly(Return(get_predefined_error()));
        EXPECT_CALL(*http_request, fake_response())
            .WillRepeatedly(Return(get_predefined_response_with_mac_only()));
    }

    std::promise<void> callback_called_success;
    std::promise<void> callback_called_busy;

    auto callback_called_success_future = callback_called_success.get_future();
    auto callback_called_busy_future = callback_called_busy.get_future();

    auto callback = make_shared<BridgeDiscoveryCallback>([&] (vector<std::shared_ptr<BridgeDiscoveryResult>> /*results*/, BridgeDiscoveryReturnCode return_code) {
        if (return_code == BRIDGE_DISCOVERY_RETURN_CODE_SUCCESS) {
            callback_called_success.set_value();
        }

        if (return_code == BRIDGE_DISCOVERY_RETURN_CODE_BUSY) {
            callback_called_busy.set_value();
        }
    });

    // Start ipscan
    _discovery_ipscan.search(callback);

    // Start ipscan again
    _discovery_ipscan.search(callback);

    callback_called_success_future.wait();
    callback_called_busy_future.wait();
}

TEST_F(TestBridgeDiscoveryIpscan, IsSearching__NoSearchInProgress__ReturnsFalse) {
    EXPECT_FALSE(_discovery_ipscan.is_searching());
}

TEST_F(TestBridgeDiscoveryIpscan, IsSearching__WhileSearchInProgress__ReturnsTrue) {
    // Expect:
    EXPECT_CALL(*_network_delegate, get_network_interfaces())
        .WillOnce(Return(get_predefined_network_interfaces()));

    for (unsigned int i = 1; i <= 254; i++) {
        string url = "http://192.168.1." + to_string(i) + huesdk::bridge_discovery_const::IPCHECK_BRIDGE_CONFIG_HTTP_URL_PATH;
        
        shared_ptr<MockHttpRequest> http_request = shared_ptr<MockHttpRequest>(new NiceMock<MockHttpRequest>(url, 0));

        // Expect:
        EXPECT_CALL(*_http_request_delegate_provider, get_delegate(url, _, _, _, _, _))
            .WillOnce(Return(http_request));
        
        // Expect:
        EXPECT_CALL(*http_request, do_get(_));
        EXPECT_CALL(*http_request, fake_delay())
            .WillRepeatedly(Return(get_predefined_delay()));
        EXPECT_CALL(*http_request, fake_error())
            .WillRepeatedly(Return(get_predefined_error()));
        EXPECT_CALL(*http_request, fake_response())
            .WillRepeatedly(Return(get_predefined_response_with_mac_only()));
    }

    std::promise<void> callback_called;
    auto callback_called_future = callback_called.get_future();

    // Start ipscan
    _discovery_ipscan.search(make_shared<BridgeDiscoveryCallback>([&] (vector<std::shared_ptr<BridgeDiscoveryResult>> /*results*/, BridgeDiscoveryReturnCode return_code) {
        EXPECT_EQ(BRIDGE_DISCOVERY_RETURN_CODE_SUCCESS, return_code);
        callback_called.set_value();
    }));


    // Check whether searching
    EXPECT_TRUE(_discovery_ipscan.is_searching());

    callback_called_future.wait();
}

TEST_F(TestBridgeDiscoveryIpscan, Stop__NoSearchInProgress__IsSearchingReturnsFalse) {
    // Check whether not searching
    EXPECT_FALSE(_discovery_ipscan.is_searching());
    
    // Stop ipscan
    _discovery_ipscan.stop();
    
    EXPECT_FALSE(_discovery_ipscan.is_searching());
}

TEST_F(TestBridgeDiscoveryIpscan, Stop__WhileSearchInProgress_AndHandlingHttpRequests_WhichWillBlockUntilFinished__CallbackCalled_IsSearchingReturnsFalse) {
    // Expect:
    EXPECT_CALL(*_network_delegate, get_network_interfaces())
            .WillOnce(Return(get_predefined_network_interfaces()));

    for (unsigned int i = 1; i <= 254; i++) {
        string url = "http://192.168.1." + to_string(i) + huesdk::bridge_discovery_const::IPCHECK_BRIDGE_CONFIG_HTTP_URL_PATH;

        shared_ptr<MockHttpRequest> http_request = shared_ptr<MockHttpRequest>(new NiceMock<MockHttpRequest>(url, 0));

        // Expect:
        EXPECT_CALL(*_http_request_delegate_provider, get_delegate(url, _, _, _, _, _))
                .Times(AtLeast(0))
                .WillRepeatedly(Return(http_request));

        // Expect:
        EXPECT_CALL(*http_request, do_get(_))
                .Times(AtLeast(0));
        EXPECT_CALL(*http_request, fake_delay())
                .Times(AtLeast(0))
                .WillRepeatedly(Return(get_predefined_delay()));
        EXPECT_CALL(*http_request, fake_error())
                .Times(AtLeast(0))
                .WillRepeatedly(Return(get_predefined_error()));
        EXPECT_CALL(*http_request, fake_response())
                .Times(AtLeast(0))
                .WillRepeatedly(Return(get_predefined_response_with_mac_only()));
    }

    atomic<bool> callback_called(false);

    // Start ipscan
    _discovery_ipscan.search(make_shared<BridgeDiscoveryCallback>([&] (vector<std::shared_ptr<BridgeDiscoveryResult>> /*results*/, BridgeDiscoveryReturnCode return_code) {
        EXPECT_EQ(BRIDGE_DISCOVERY_RETURN_CODE_STOPPED, return_code);
        callback_called = true;
    }));

    // Wait a bit before stopping the search
    std::this_thread::sleep_for(milliseconds(75));

    // Stop ipscan
    _discovery_ipscan.stop();

    // Wait a bit to ensure the callback is called
    std::this_thread::sleep_for(milliseconds(1000));

    EXPECT_TRUE(callback_called);
    EXPECT_FALSE(_discovery_ipscan.is_searching());
}

TEST_F(TestBridgeDiscoveryIpscan, PreCheckFiltersReachableIps_Success) {
    SOCKET_ID last_fd = 0;
    int mock_errno = SOCKET_INPROGRESS;

    EXPECT_CALL(*_cmethod_delegate, socket(_, _, _))
            .WillRepeatedly(Invoke([&](int /*domain*/, int /*type*/, int /*protocol*/) -> SOCKET_ID {
                last_fd++;

                if (last_fd == 3) {
                    return INVALID_SOCKET;
                }

                return last_fd;
            }));

    EXPECT_CALL(*_cmethod_delegate, connect(_, _, _))
            .WillRepeatedly(Invoke([&](SOCKET_ID fd, const struct sockaddr* /*address*/, socklen_t /*address_len*/) -> int {
                if (fd == 6) {
                    return 0;
                }

                if (fd == 2) {
                    // some unknown error
                    mock_errno = 999999;
                } else {
                    mock_errno = SOCKET_INPROGRESS;
                }

                return -1;
            }));

    EXPECT_CALL(*_cmethod_delegate, get_errno())
            .WillRepeatedly(Invoke([&]() -> int {
                return mock_errno;
            }));

    EXPECT_CALL(*_cmethod_delegate, set_nonblocking(_))
            .WillRepeatedly(Invoke([&](SOCKET_ID fd) -> int {
                if (fd == 9) {
                    return -1;
                }
                return 0;
            }));

    EXPECT_CALL(*_cmethod_delegate, inet_pton(_, _, _))
            .WillRepeatedly(Invoke([&](int /*af*/, const char* src, void* /*dst*/) -> int {
                if (string("192.168.1.8") == src) {
                    return 0;
                }
                return 1;
            }));

    int iteration = 0;

    EXPECT_CALL(*_cmethod_delegate, select(_, _, _, _, _))
            .WillRepeatedly(Invoke([&](SOCKET_ID /*handle*/, fd_set* read_flags, fd_set* write_flags, fd_set* error_flags, struct timeval* /*timeout*/) -> int {
                iteration++;

                if (iteration == 1) {
                    return -1;
                }

                FD_ZERO(read_flags);
                FD_ZERO(write_flags);
                FD_ZERO(error_flags);

                FD_SET(1, write_flags);
                FD_SET(2, write_flags);
                // 3, unused
                FD_SET(4, read_flags);
                // 5, timeout
                FD_SET(6, write_flags);
                FD_SET(7, error_flags);
                FD_SET(8, write_flags);
                FD_SET(9, write_flags);

                return 0;
            }));

    BridgeDiscoveryIpscanPreCheck prechecker;

    std::vector<std::string> ips = {
        "192.168.1.9",  // fd 9, will fail during set_nonblocking
        "192.168.1.8",  // fd 8, will fail during inet_pton
        "192.168.1.7",  // fd 7, will fail to connect (socket error)
        "192.168.1.6",  // fd 6, will connect immediately
        "192.168.1.5",  // fd 5, will timeout
        "192.168.1.4",  // fd 4, will fail to connect (closed port)
                        // fd 3, will error when calling socket()
        "192.168.1.2",  // fd 2, will fail to connect
        "192.168.1.1"   // fd 1, will succeed
    };

    auto filtered_ips = prechecker.filter_reachable_ips(ips, std::atomic<bool>());

    EXPECT_THAT(filtered_ips, ElementsAre(string("192.168.1.1"), string("192.168.1.6")));
}
