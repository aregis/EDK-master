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

#include "support/network/NetworkInterface.h"
#include "support/network/sockets/SocketUdp.h"
#include "support/network/sockets/_test/SocketUdpDelegator.h"
#include "support/network/sockets/SocketAddress.h"
#include "support/util/VectorOperations.h"

#include "method/upnp/BridgeDiscoveryUpnp.h"
#include "bridgediscovery/BridgeDiscovery.h"
#include "bridgediscovery/BridgeDiscoveryResult.h"

#include "support/mock/network/sockets/MockSocketUdp.h"
#include "support/mock/network/sockets/MockSocketUdpDelegateProvider.h"
#include "support/mock/network/MockNetworkDelegate.h"

using std::string;
using std::vector;
using std::make_shared;
using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::steady_clock;
using huesdk::BridgeDiscoveryCallback;
using huesdk::BridgeDiscoveryUpnp;
using huesdk::BridgeDiscoveryResult;
using huesdk::BridgeDiscoveryReturnCode;
using huesdk::BRIDGE_DISCOVERY_RETURN_CODE_BUSY;
using huesdk::BRIDGE_DISCOVERY_RETURN_CODE_SUCCESS;
using huesdk::BRIDGE_DISCOVERY_RETURN_CODE_STOPPED;
using support::INET_IPV4;
using support::NetworkDelegateImpl;
using support::NetworkDelegator;
using support::SOCKET_STATUS_FAILED;
using support::SOCKET_STATUS_OK;
using support::SocketAddress;
using support::SocketError;
using support::SocketUdpDelegatorProviderImpl;
using support::SocketUdpDelegator;
using support_unittests::MockNetworkDelegate;
using support_unittests::MockSocketUdp;
using support_unittests::MockSocketUdpDelegateProvider;
using testing::AtLeast;
using testing::NiceMock;
using testing::Return;
using testing::Sequence;
using testing::StrictMock;
using testing::Test;


/* fixture */

class TestBridgeDiscoveryUpnp : public Test {
protected:
    /**
    
     */
    virtual void SetUp() {
        _network_delegate = shared_ptr<MockNetworkDelegate>(new StrictMock<MockNetworkDelegate>());
        // Inject the network delegate into the network delegator
        NetworkDelegator::set_delegate(_network_delegate);
        
        _socket_delegate_provider = shared_ptr<MockSocketUdpDelegateProvider>(new StrictMock<MockSocketUdpDelegateProvider>());
        // Inject the delegate provider into the udp socket delegator
        SocketUdpDelegator::set_delegate_provider(_socket_delegate_provider);
        
        _socket = shared_ptr<MockSocketUdp>(new NiceMock<MockSocketUdp>(SocketAddress("239.255.255.250", 1900)));
    }

    /** 
    
     */
    virtual void TearDown() {
        // Reset network delegate to it's original value
        NetworkDelegator::set_delegate(shared_ptr<NetworkDelegate>(new NetworkDelegateImpl()));
        // Reset delegate to it's original value
        SocketUdpDelegator::set_delegate_provider(shared_ptr<SocketUdpDelegatorProvider>(new SocketUdpDelegatorProviderImpl()));
        
        _socket = nullptr;
    }
    
    /**
    
     */
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
    unsigned int get_predefined_sending_delay() {
        // Default delay is 100 ms
        return 100;
    }
    
    /**
    
     */
    SocketError get_predefined_sending_error() {
        SocketError error;
        
        error.set_code(SOCKET_STATUS_OK);
        
        return error;
    }

    /**
    
     */
    unsigned int get_predefined_receiving_delay() {
        // Default delay is 100 ms
        return 100;
    }
    
    /**
    
     */
    SocketError get_predefined_receiving_error() {
        SocketError error;
        
        error.set_code(SOCKET_STATUS_OK);
        error.set_message("");
        
        return error;
    }

    /**
    
     */
    string get_predefined_receiving_data() {
        return "HTTP/1.1 200 OK\r\n"\
               "CACHE-CONTROL: max-age=100\r\n"\
               "EXT:\r\n"\
               "LOCATION: http://192.168.2.1:80/description.xml\r\n"\
               "SERVER: FreeRTOS/6.0.5, "\
               "UPnP/1.0, "\
               "IpBridge/0.1\r\n"\
               "ST: upnp:rootdevice\r\n"\
               "USN: uuid:2f402f80-da50-11e1-9b23-0017881055ec::upnp:rootdevice\r\n\r\n"\
               "HTTP/1.1 200 OK\r\n"\
               "CACHE-CONTROL: max-age=100\r\n"\
               "EXT:\r\n"\
               "LOCATION: http://192.168.2.2:80/description.xml\r\n"\
               "SERVER: FreeRTOS/6.0.5, "\
               "UPnP/1.0, "\
               "IpBridge/0.1\r\n"\
               "ST: upnp:rootdevice\r\n"\
               "USN: uuid:2f402f80-da50-11e1-9b23-0017881055ed::upnp:rootdevice\r\n"\
               "hue-bridgeid: 001788FFFE1055EZ\r\n\r\n"\
               "HTTP/1.1 200 OK\r\n"\
               "CACHE-CONTROL: max-age=100\r\n"\
               "EXT:\r\n"\
               "LOCATION: http://192.168.2.3:80/description.xml\r\n"\
               "SERVER: FreeRTOS/6.0.5, "\
               "UPnP/1.0, "\
               "IpBridge/0.1\r\n"\
               "ST: upnp:rootdevice\r\n"\
               "USN: uuid:2f402f80-da50-11e1-9b23-0017881055ee::upnp:rootdevice\r\n\r\n"\
               "NOTIFY * HTTP/1.1\r\n"\
               "LOCATION: http://192.168.2.4:80/description.xml\r\n"\
               "SERVER: FreeRTOS/6.0.5, "\
               "UPnP/1.0, "\
               "IpBridge/1.9.0\r\n"\
               "hue-bridgeid: 001788FFFE0AE670\r\n"\
               "ST: upnp:rootdevice\r\n"\
               "USN: uuid:2f402f80-da50-11e1-9b23-bladiebla::upnp:rootdevice";
    }

    shared_ptr<MockNetworkDelegate>           _network_delegate;
    /** */
    shared_ptr<MockSocketUdp>                 _socket;
    /** */
    shared_ptr<MockSocketUdpDelegateProvider> _socket_delegate_provider;
};


TEST_F(TestBridgeDiscoveryUpnp, Search_WithCallback__SocketInitialized_AndDiscoveryMessageSent_AndValidResponseData__CallbackCalledWith4Bridges) {
    // Expect:
    EXPECT_CALL(*_network_delegate, get_network_interfaces())
        .WillOnce(Return(get_predefined_network_interfaces()));
    // Expect: get_delegate will be called once and the mocked udp socket will be returned
    EXPECT_CALL(*_socket_delegate_provider, get_delegate(_))
        .WillOnce(Return(_socket));
    // Expect:
    EXPECT_CALL(*_socket, async_send(_, _, _));
    // Expect:
    EXPECT_CALL(*_socket, async_receive(_, _, _))
        .Times(AtLeast(1));

    EXPECT_CALL(*_socket, fake_sending_delay())
        .WillRepeatedly(Return(get_predefined_sending_delay()));
    EXPECT_CALL(*_socket, fake_sending_error())
        .WillRepeatedly(Return(get_predefined_sending_error()));
    EXPECT_CALL(*_socket, fake_receiving_delay())
        .WillRepeatedly(Return(get_predefined_receiving_delay()));
    EXPECT_CALL(*_socket, fake_receiving_error())
        .WillRepeatedly(Return(get_predefined_receiving_error()));

    Sequence seq;
    // First call return the predefined data
    EXPECT_CALL(*_socket, fake_receiving_data())
        .InSequence(seq)
        .WillOnce(Return(get_predefined_receiving_data()));
    // By default return empty data
    EXPECT_CALL(*_socket, fake_receiving_data())
        .InSequence(seq)
        .WillRepeatedly(Return(""));

    std::promise<void> callback_called;

    // Start upnp search
    BridgeDiscoveryUpnp discovery_upnp;
    discovery_upnp.search(make_shared<BridgeDiscoveryCallback>([&] (vector<std::shared_ptr<BridgeDiscoveryResult>> results, BridgeDiscoveryReturnCode return_code) {
        EXPECT_STREQ("001788FFFE1055EC", results[0]->get_unique_id());
        EXPECT_STREQ("192.168.2.1",      results[0]->get_ip());
        EXPECT_STREQ("0.1",              results[0]->get_api_version());
        EXPECT_STREQ("001788FFFE1055EZ", results[1]->get_unique_id());
        EXPECT_STREQ("192.168.2.2",      results[1]->get_ip());
        EXPECT_STREQ("0.1",              results[1]->get_api_version());
        EXPECT_STREQ("001788FFFE1055EE", results[2]->get_unique_id());
        EXPECT_STREQ("192.168.2.3",      results[2]->get_ip());
        EXPECT_STREQ("0.1",              results[2]->get_api_version());
        EXPECT_STREQ("001788FFFE0AE670", results[3]->get_unique_id());
        EXPECT_STREQ("192.168.2.4",      results[3]->get_ip());
        EXPECT_STREQ("1.9.0",            results[3]->get_api_version());
        EXPECT_EQ(BRIDGE_DISCOVERY_RETURN_CODE_SUCCESS, return_code);

        callback_called.set_value();
    }));

    callback_called.get_future().wait();
}

TEST_F(TestBridgeDiscoveryUpnp, Search_WithCallback__SocketInitialized_AndDiscoveryMessageSent_AndEmptyResponseData__CallbackCalledWithNoBridges) {
    // Expect:
    EXPECT_CALL(*_network_delegate, get_network_interfaces())
        .WillOnce(Return(get_predefined_network_interfaces()));
    // Expect: get_delegate will be called once and the mocked udp socket will be returned
    EXPECT_CALL(*_socket_delegate_provider, get_delegate(_))
        .WillOnce(Return(_socket));
    // Expect:
    EXPECT_CALL(*_socket, async_send(_, _, _));
    // Expect:
    EXPECT_CALL(*_socket, async_receive(_, _, _))
        .Times(AtLeast(1));

    EXPECT_CALL(*_socket, fake_sending_delay())
        .WillRepeatedly(Return(get_predefined_sending_delay()));
    EXPECT_CALL(*_socket, fake_sending_error())
        .WillRepeatedly(Return(get_predefined_sending_error()));
    EXPECT_CALL(*_socket, fake_receiving_delay())
        .WillRepeatedly(Return(get_predefined_receiving_delay()));
    EXPECT_CALL(*_socket, fake_receiving_error())
        .WillRepeatedly(Return(get_predefined_receiving_error()));
    EXPECT_CALL(*_socket, fake_receiving_data())
        .WillRepeatedly(Return(""));

    std::promise<void> callback_called;


    // Start upnp search
    BridgeDiscoveryUpnp discovery_upnp;
    discovery_upnp.search(make_shared<BridgeDiscoveryCallback>([&] (vector<std::shared_ptr<BridgeDiscoveryResult>> results, BridgeDiscoveryReturnCode return_code) {
        EXPECT_EQ(0ul,       results.size());
        EXPECT_EQ(BRIDGE_DISCOVERY_RETURN_CODE_SUCCESS, return_code);

        callback_called.set_value();
    }));

    callback_called.get_future().wait();
}

TEST_F(TestBridgeDiscoveryUpnp, Search_WithCallback__SocketInitialized_AndDiscoveryMessageSent_AndInvalidResponseData__CallbackCalledWithNoBridges) {
    // Expect:
    EXPECT_CALL(*_network_delegate, get_network_interfaces())
        .WillOnce(Return(get_predefined_network_interfaces()));
    // Expect: get_delegate will be called once and the mocked udp socket will be returned
    EXPECT_CALL(*_socket_delegate_provider, get_delegate(_))
        .WillOnce(Return(_socket));
    // Expect:
    EXPECT_CALL(*_socket, async_send(_, _, _));
    // Expect:
    EXPECT_CALL(*_socket, async_receive(_, _, _))
        .Times(AtLeast(1));

    EXPECT_CALL(*_socket, fake_sending_delay())
        .WillRepeatedly(Return(get_predefined_sending_delay()));
    EXPECT_CALL(*_socket, fake_sending_error())
        .WillRepeatedly(Return(get_predefined_sending_error()));
    EXPECT_CALL(*_socket, fake_receiving_delay())
        .WillRepeatedly(Return(get_predefined_receiving_delay()));
    EXPECT_CALL(*_socket, fake_receiving_error())
        .WillRepeatedly(Return(get_predefined_receiving_error()));

    Sequence seq;
    // First call return the predefined data
    EXPECT_CALL(*_socket, fake_receiving_data())
        .InSequence(seq)
        .WillOnce(Return("HTTP/1.1 200 OK\r\n"\
                         "CACHE-CONTROL: max-age=100\r\n"\
                         "EXT:\r\n"\
                         "LOCATION: http://invalidip.com/description.xml\r\n"\
                         "SERVER: FreeRTOS/6.0.5, "\
                         "UPnP/1.0, "\
                         "IpBridge/0.1\r\n"\
                         "ST: upnp:rootdevice\r\n"\
                         "USN: uuid:2f402f80-da50-11e1-9b23-invalidmac::upnp:rootdevice\r\n\r\n"));
    // By default return empty data
    EXPECT_CALL(*_socket, fake_receiving_data())
        .InSequence(seq)
        .WillRepeatedly(Return(""));

    std::promise<void> callback_called;

    // Start upnp search
    BridgeDiscoveryUpnp discovery_upnp;
    discovery_upnp.search(make_shared<BridgeDiscoveryCallback>([&] (vector<std::shared_ptr<BridgeDiscoveryResult>> results, BridgeDiscoveryReturnCode return_code) {
        EXPECT_EQ(0ul,       results.size());
        EXPECT_EQ(BRIDGE_DISCOVERY_RETURN_CODE_SUCCESS, return_code);

        callback_called.set_value();
    }));

    callback_called.get_future().wait();
}

TEST_F(TestBridgeDiscoveryUpnp, Search_WithCallback__SocketInitialized_AndDiscoveryMessageSent_AndPartialInvalidResponseData__CallbackCalledWithNoBridges) {
    // Expect:
    EXPECT_CALL(*_network_delegate, get_network_interfaces())
        .WillOnce(Return(get_predefined_network_interfaces()));
    // Expect: get_delegate will be called once and the mocked udp socket will be returned
    EXPECT_CALL(*_socket_delegate_provider, get_delegate(_))
        .WillOnce(Return(_socket));
    // Expect:
    EXPECT_CALL(*_socket, async_send(_, _, _));
    // Expect:
    EXPECT_CALL(*_socket, async_receive(_, _, _))
        .Times(AtLeast(1));
    EXPECT_CALL(*_socket, fake_sending_delay())
        .WillRepeatedly(Return(get_predefined_sending_delay()));
    EXPECT_CALL(*_socket, fake_sending_error())
        .WillRepeatedly(Return(get_predefined_sending_error()));
    EXPECT_CALL(*_socket, fake_receiving_delay())
        .WillRepeatedly(Return(get_predefined_receiving_delay()));
    EXPECT_CALL(*_socket, fake_receiving_error())
        .WillRepeatedly(Return(get_predefined_receiving_error()));

    Sequence seq;
    // First call return the predefined data
    EXPECT_CALL(*_socket, fake_receiving_data())
        .InSequence(seq)
        .WillOnce(Return("HTTP/1.1 200 OK\r\n"\
                         "CACHE-CONTROL: max-age=100\r\n"\
                         "EXT:\r\n"\
                         "LOCATION: http://192.168.2.1:80/description.xml\r\n"\
                         "SERVER: FreeRTOS/6.0.5, "\
                         "UPnP/1.0, "\
                         "IpBridge/0.1\r\n"\
                         "ST: upnp:rootdevice\r\n"\
                         "USN: uuid:2f402f80-da50-11e1-9b23-0017881055ec::upnp:rootdevice\r\n\r\n"\
                         "HTTP/1.1 200 OK\r\n"\
                         "CACHE-CONTROL: max-age=100\r\n"\
                         "EXT:\r\n"\
                         "LOCATION: http://192.168.2.2:80/description.xml\r\n"\
                         "SERVER: FreeRTOS/6.0.5, "\
                         "UPnP/1.0, "\
                         "IpBridge/0.1\r\n"\
                         "ST: upnp:rootdevice\r\n"\
                         "USN: uuid:2f402f80-da50-11e1-9b23-0017881055ed::upnp:rootdevice\r\n\r\n"\
                         "HTTP/1.1 200 OK\r\n"\
                         "CACHE-CONTROL: max-age=100\r\n"\
                         "EXT:\r\n"\
                         "LOCATION: http://invalidip.com/description.xml\r\n"\
                         "SERVER: FreeRTOS/6.0.5, "\
                         "UPnP/1.0, "\
                         "IpBridge/0.1\r\n"\
                         "ST: upnp:rootdevice\r\n"\
                         "USN: uuid:2f402f80-da50-11e1-9b23-invalidmac::upnp:rootdevice\r\n\r\n"));
    // By default return empty data
    EXPECT_CALL(*_socket, fake_receiving_data())
        .InSequence(seq)
        .WillRepeatedly(Return(""));

    std::promise<void> callback_called;

    // Start upnp search
    BridgeDiscoveryUpnp discovery_upnp;
    discovery_upnp.search(make_shared<BridgeDiscoveryCallback>([&] (vector<std::shared_ptr<BridgeDiscoveryResult>> results, BridgeDiscoveryReturnCode return_code) {
        ASSERT_EQ(2ul, results.size());

        EXPECT_STREQ("001788FFFE1055EC", results[0]->get_unique_id());
        EXPECT_STREQ("192.168.2.1",      results[0]->get_ip());
        EXPECT_STREQ("0.1",              results[0]->get_api_version());
        EXPECT_STREQ("001788FFFE1055ED", results[1]->get_unique_id());
        EXPECT_STREQ("192.168.2.2",      results[1]->get_ip());
        EXPECT_STREQ("0.1",              results[1]->get_api_version());
        EXPECT_EQ(BRIDGE_DISCOVERY_RETURN_CODE_SUCCESS, return_code);

        callback_called.set_value();
    }));

    callback_called.get_future().wait();
}

TEST_F(TestBridgeDiscoveryUpnp, Search_WithCallback__SocketInitialized_AndDiscoveryMessageSent_AndErrorResponse__CallbackCalledWithNoBridges) {
    // Expect:
    EXPECT_CALL(*_network_delegate, get_network_interfaces())
        .WillOnce(Return(get_predefined_network_interfaces()));
    // Expect: get_delegate will be called once and the mocked udp socket will be returned
    EXPECT_CALL(*_socket_delegate_provider, get_delegate(_))
        .WillOnce(Return(_socket));
    // Expect:
    EXPECT_CALL(*_socket, async_send(_, _, _));
    // Expect:
    EXPECT_CALL(*_socket, async_receive(_, _, _))
        .Times(AtLeast(1));

    EXPECT_CALL(*_socket, fake_sending_delay())
        .WillRepeatedly(Return(get_predefined_sending_delay()));
    EXPECT_CALL(*_socket, fake_sending_error())
        .WillRepeatedly(Return(get_predefined_sending_error()));
    EXPECT_CALL(*_socket, fake_receiving_delay())
        .WillRepeatedly(Return(get_predefined_receiving_delay()));
    EXPECT_CALL(*_socket, fake_receiving_error())
        .WillRepeatedly(Return(SocketError("", SOCKET_STATUS_FAILED)));
    EXPECT_CALL(*_socket, fake_receiving_data())
        .WillRepeatedly(Return(""));

    std::promise<void> callback_called;

    // Start upnp search
    BridgeDiscoveryUpnp discovery_upnp;
    discovery_upnp.search(make_shared<BridgeDiscoveryCallback>([&] (vector<std::shared_ptr<BridgeDiscoveryResult>> results, BridgeDiscoveryReturnCode return_code) {
        EXPECT_EQ(0ul,       results.size());
        EXPECT_EQ(BRIDGE_DISCOVERY_RETURN_CODE_SUCCESS, return_code);

        callback_called.set_value();
    }));

    callback_called.get_future().wait();
}

TEST_F(TestBridgeDiscoveryUpnp, Search_WithCallback__SocketInitialized_AndErrorWhileSendingDiscoveryMessage__CallbackCalledWithNoBridges) {
    // Expect:
    EXPECT_CALL(*_network_delegate, get_network_interfaces())
        .WillOnce(Return(get_predefined_network_interfaces()));
    // Expect: get_delegate will be called once and the mocked udp socket will be returned
    EXPECT_CALL(*_socket_delegate_provider, get_delegate(_))
        .WillOnce(Return(_socket));
    // Expect:
    EXPECT_CALL(*_socket, async_send(_, _, _));
    // Expect:
    EXPECT_CALL(*_socket, async_receive(_, _, _))
        .Times(0);
    EXPECT_CALL(*_socket, fake_sending_delay())
        .WillRepeatedly(Return(get_predefined_sending_delay()));
    EXPECT_CALL(*_socket, fake_sending_error())
        .WillRepeatedly(Return(SocketError("", SOCKET_STATUS_FAILED)));

    std::promise<void> callback_called;

    // Start upnp search
    BridgeDiscoveryUpnp discovery_upnp;
    discovery_upnp.search(make_shared<BridgeDiscoveryCallback>([&] (vector<std::shared_ptr<BridgeDiscoveryResult>> results, BridgeDiscoveryReturnCode return_code) {
        EXPECT_EQ(0ul,       results.size());
        EXPECT_EQ(BRIDGE_DISCOVERY_RETURN_CODE_SUCCESS, return_code);

        callback_called.set_value();
    }));

    callback_called.get_future().wait();
}

TEST_F(TestBridgeDiscoveryUpnp, Search_WithCallback__NoValidNetworkInterface__CallbackCalledWithNoBridges) {
    // Expect:
    EXPECT_CALL(*_network_delegate, get_network_interfaces())
        .WillOnce(Return(std::vector<NetworkInterface>()));
    EXPECT_CALL(*_socket_delegate_provider, get_delegate(_))
        .Times(0);

    std::promise<void> callback_called;

    // Start upnp search
    BridgeDiscoveryUpnp discovery_upnp;
    discovery_upnp.search(make_shared<BridgeDiscoveryCallback>([&] (vector<std::shared_ptr<BridgeDiscoveryResult>> results, BridgeDiscoveryReturnCode return_code) {
        EXPECT_EQ(0ul,       results.size());
        EXPECT_EQ(BRIDGE_DISCOVERY_RETURN_CODE_SUCCESS, return_code);

        callback_called.set_value();
    }));

    callback_called.get_future().wait();
}

TEST_F(TestBridgeDiscoveryUpnp, Search_WithCallback__SocketErrorOccurred__CallbackCalledWithNoBridges) {
    // Expect:
    EXPECT_CALL(*_network_delegate, get_network_interfaces())
        .WillOnce(Return(get_predefined_network_interfaces()));
    // Expect: get_delegate will be called once and the mocked udp socket will be returned
    EXPECT_CALL(*_socket_delegate_provider, get_delegate(_))
        .WillOnce(Return(_socket));
    // Expect:
    EXPECT_CALL(*_socket, bind())
        .WillOnce(Return(SOCKET_STATUS_FAILED));
    // Expect:
    EXPECT_CALL(*_socket, async_send(_, _, _))
        .Times(0);
    // Expect:
    EXPECT_CALL(*_socket, async_receive(_, _, _))
        .Times(0);

    std::promise<void> callback_called;

    // Start upnp search
    BridgeDiscoveryUpnp discovery_upnp;
    discovery_upnp.search(make_shared<BridgeDiscoveryCallback>([&] (vector<std::shared_ptr<BridgeDiscoveryResult>> results, BridgeDiscoveryReturnCode return_code) {
        EXPECT_EQ(0ul,       results.size());
        EXPECT_EQ(BRIDGE_DISCOVERY_RETURN_CODE_SUCCESS, return_code);

        callback_called.set_value();
    }));

    callback_called.get_future().wait();
}

TEST_F(TestBridgeDiscoveryUpnp, Search_WithCallback__WhileAlreadySearchInProgress__ReturnsError) {
    // Expect:
    EXPECT_CALL(*_network_delegate, get_network_interfaces())
        .WillOnce(Return(get_predefined_network_interfaces()));
    // Expect: get_delegate will be called once and the mocked udp socket will be returned
    EXPECT_CALL(*_socket_delegate_provider, get_delegate(_))
        .WillOnce(Return(_socket));
    // Expect:
    EXPECT_CALL(*_socket, async_send(_, _, _));
    // Expect:
    EXPECT_CALL(*_socket, async_receive(_, _, _))
        .Times(AtLeast(1));

    EXPECT_CALL(*_socket, fake_sending_delay())
        .WillRepeatedly(Return(get_predefined_sending_delay()));
    EXPECT_CALL(*_socket, fake_sending_error())
        .WillRepeatedly(Return(get_predefined_sending_error()));
    EXPECT_CALL(*_socket, fake_receiving_delay())
        .WillRepeatedly(Return(get_predefined_receiving_delay()));
    EXPECT_CALL(*_socket, fake_receiving_error())
        .WillRepeatedly(Return(get_predefined_receiving_error()));
    EXPECT_CALL(*_socket, fake_receiving_data())
        .WillOnce(Return(get_predefined_receiving_data()))
        .WillRepeatedly(Return(""));

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

    // Start upnp search
    BridgeDiscoveryUpnp discovery_upnp;
    discovery_upnp.search(callback);
    // Start upnp search again
    discovery_upnp.search(callback);

    callback_called_success.get_future().wait();
    callback_called_busy.get_future().wait();
}

TEST_F(TestBridgeDiscoveryUpnp, Search_WithoutCallback__ReturnsError) {
    BridgeDiscoveryUpnp discovery_upnp;
    discovery_upnp.search(nullptr);
}

TEST_F(TestBridgeDiscoveryUpnp, IsSearching__NoSearchInProgress__ReturnsFalse) {
    BridgeDiscoveryUpnp discovery_upnp;
    EXPECT_FALSE(discovery_upnp.is_searching());
}

TEST_F(TestBridgeDiscoveryUpnp, IsSearching__WhileSearchInProgress__ReturnsTrue) {
    // Expect:
    EXPECT_CALL(*_network_delegate, get_network_interfaces())
        .WillOnce(Return(get_predefined_network_interfaces()));
    // Expect: get_delegate will be called once and the mocked udp socket will be returned
    EXPECT_CALL(*_socket_delegate_provider, get_delegate(_))
        .WillOnce(Return(_socket));
    // Expect:
    EXPECT_CALL(*_socket, async_send(_, _, _));
    // Expect:
    EXPECT_CALL(*_socket, async_receive(_, _, _))
        .Times(AtLeast(1));

    EXPECT_CALL(*_socket, fake_sending_delay())
        .WillRepeatedly(Return(get_predefined_sending_delay()));
    EXPECT_CALL(*_socket, fake_sending_error())
        .WillRepeatedly(Return(get_predefined_sending_error()));
    EXPECT_CALL(*_socket, fake_receiving_delay())
        .WillRepeatedly(Return(get_predefined_receiving_delay()));
    EXPECT_CALL(*_socket, fake_receiving_error())
        .WillRepeatedly(Return(get_predefined_receiving_error()));
    EXPECT_CALL(*_socket, fake_receiving_data())
        .WillOnce(Return(get_predefined_receiving_data()))
        .WillRepeatedly(Return(""));

    std::promise<void> callback_called;

    // Start upnp search
    BridgeDiscoveryUpnp discovery_upnp;
    discovery_upnp.search(make_shared<BridgeDiscoveryCallback>([&] (vector<std::shared_ptr<BridgeDiscoveryResult>> /*results*/, BridgeDiscoveryReturnCode return_code) {
        EXPECT_EQ(BRIDGE_DISCOVERY_RETURN_CODE_SUCCESS, return_code);

        callback_called.set_value();
    }));

    // Check whether searching
    EXPECT_TRUE(discovery_upnp.is_searching());

    callback_called.get_future().wait();
}

TEST_F(TestBridgeDiscoveryUpnp, Stop__NoSearchInProgress__IsSearchingReturnsFalse) {
    BridgeDiscoveryUpnp discovery_upnp;
    // Check whether not searching
    EXPECT_FALSE(discovery_upnp.is_searching());

    // Stop upnp search
    discovery_upnp.stop();

    EXPECT_FALSE(discovery_upnp.is_searching());
}

TEST_F(TestBridgeDiscoveryUpnp, Stop__WhileSearchInProgress_AndSendingDiscoveryMessage__CallbackCalled_IsSearchingReturnsFalse) {
    // Expect:
    EXPECT_CALL(*_network_delegate, get_network_interfaces())
        .WillOnce(Return(get_predefined_network_interfaces()));
    // Expect: get_delegate will be called once and the mocked udp socket will be returned
    EXPECT_CALL(*_socket_delegate_provider, get_delegate(_))
        .WillOnce(Return(_socket));
    // Expect:
    EXPECT_CALL(*_socket, async_send(_, _, _));
    // Expect:
    EXPECT_CALL(*_socket, async_receive(_, _, _))
        .Times(0);
    EXPECT_CALL(*_socket, fake_sending_delay())
        .WillRepeatedly(Return(2000));
    EXPECT_CALL(*_socket, fake_sending_error())
        .WillRepeatedly(Return(get_predefined_sending_error()));

    std::atomic<bool> callback_called;

    // Start nupnp search
    BridgeDiscoveryUpnp discovery_upnp;
    discovery_upnp.search(make_shared<BridgeDiscoveryCallback>([&] (vector<std::shared_ptr<BridgeDiscoveryResult>> /*results*/, BridgeDiscoveryReturnCode return_code) {
        EXPECT_EQ(BRIDGE_DISCOVERY_RETURN_CODE_STOPPED, return_code);

        callback_called = true;
    }));

    // Wait a bit before stopping the search
    std::this_thread::sleep_for(milliseconds(1000));

    // Stop nupnp search
    discovery_upnp.stop();

    // Wait a bit to ensure the callback is called
    std::this_thread::sleep_for(milliseconds(1000));

    EXPECT_TRUE(callback_called.load());
    // Check whether the search stopped in an acceptable timespan

    EXPECT_FALSE(discovery_upnp.is_searching());
}

TEST_F(TestBridgeDiscoveryUpnp, Stop__WhileSearchInProgress_AndDiscoveryMessageSent_AndReceivingResponseData__CallbackCalled_IsSearchingReturnsFalse) {
    // Expect:
    EXPECT_CALL(*_network_delegate, get_network_interfaces())
        .WillOnce(Return(get_predefined_network_interfaces()));
    // Expect: get_delegate will be called once and the mocked udp socket will be returned
    EXPECT_CALL(*_socket_delegate_provider, get_delegate(_))
        .WillOnce(Return(_socket));
    // Expect:
    EXPECT_CALL(*_socket, async_send(_, _, _));
    // Expect:
    EXPECT_CALL(*_socket, async_receive(_, _, _))
        .Times(AtLeast(1));

    EXPECT_CALL(*_socket, fake_sending_delay())
        .WillRepeatedly(Return(get_predefined_sending_delay()));
    EXPECT_CALL(*_socket, fake_sending_error())
        .WillRepeatedly(Return(get_predefined_sending_error()));
    EXPECT_CALL(*_socket, fake_receiving_delay())
        .WillRepeatedly(Return(get_predefined_receiving_delay()));
    EXPECT_CALL(*_socket, fake_receiving_error())
        .WillRepeatedly(Return(get_predefined_receiving_error()));
    EXPECT_CALL(*_socket, fake_receiving_data())
        .WillOnce(Return(get_predefined_receiving_data()))
        .WillRepeatedly(Return(""));

    std::atomic<bool> callback_called;

    // Start nupnp search
    BridgeDiscoveryUpnp discovery_upnp;
    discovery_upnp.search(make_shared<BridgeDiscoveryCallback>([&] (vector<std::shared_ptr<BridgeDiscoveryResult>> /*results*/, BridgeDiscoveryReturnCode return_code) {
        EXPECT_EQ(BRIDGE_DISCOVERY_RETURN_CODE_STOPPED, return_code);

        callback_called = true;
    }));

    // Wait a bit before stopping the search
    std::this_thread::sleep_for(milliseconds(2500));

    // Stop nupnp search
    discovery_upnp.stop();

    // Wait a bit to ensure the callback is called
    std::this_thread::sleep_for(milliseconds(1000));

    EXPECT_TRUE(callback_called.load());
    // Check whether the search stopped in an acceptable timespan

    EXPECT_FALSE(discovery_upnp.is_searching());
}
