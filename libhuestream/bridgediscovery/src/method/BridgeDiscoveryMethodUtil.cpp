/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <condition_variable>
#include <mutex>
#include <string>
#include <regex>
#include <vector>

#include "libjson/libjson.h"
#include "support/logging/Log.h"
#include "support/network/http/IHttpResponse.h"
#include "support/network/http/HttpRequestError.h"

#include "support/network/NetworkInterface.h"
#include "support/network/_test/NetworkDelegator.h"
#include "support/network/http/_test/HttpRequestDelegator.h"

using support::HTTP_REQUEST_STATUS_OK;

#define NETWORK      support::NetworkDelegator
#define HTTP_REQUEST support::HttpRequestDelegator

#include "bridgediscovery/BridgeDiscoveryConst.h"
#include "bridgediscovery/BridgeInfo.h"
#include "bridgediscovery/BridgeDiscoveryConfiguration.h"

#include "method/BridgeDiscoveryMethodUtil.h"

using std::condition_variable;
using std::mutex;
using std::string;
using std::smatch;
using std::regex;
using std::vector;
using support::IHttpResponse;
using support::HttpRequestError;
using support::INET_IPV4;
using support::NetworkInterface;
using support::prioritize;

namespace huesdk {

using IpCheckFutureVector = std::vector<std::future<BridgeDiscoveryIpCheckResult>>;

    unique_ptr<NetworkInterface> BridgeDiscoveryMethodUtil::get_first_private_network_interface() {
        unique_ptr<NetworkInterface> network_interface;
        
        HUE_LOG << HUE_CORE << HUE_DEBUG << "BridgeDiscoveryMethodUtil: get network interfaces" << HUE_ENDL;

        // Get all network interfaces
        const std::vector<NetworkInterface>& network_interfaces = NETWORK::get_network_interfaces();
        
        HUE_LOG << HUE_CORE << HUE_DEBUG << "BridgeDiscoveryMethodUtil: network interfaces retrieved; check results" << HUE_ENDL;

        for (NetworkInterface network_interface_it : prioritize(network_interfaces)) {
            HUE_LOG << HUE_CORE << HUE_DEBUG << "BridgeDiscoveryMethodUtil: ip: "
                    << network_interface_it.get_ip() << ", name: " << network_interface_it.get_name()
                    << " isWifi " << (network_interface_it.get_adapter_type() == support::NetworkAdapterType::NETWORK_ADAPTER_TYPE_WIRELESS)
                    << " isConnected " << network_interface_it.get_is_connected()
                    << HUE_ENDL;
        }

        for (NetworkInterface network_interface_it : prioritize(network_interfaces)) {
            if (network_interface_it.get_inet_type() == INET_IPV4
                && !network_interface_it.is_loopback()
                && network_interface_it.is_up()
#ifdef __APPLE__
                &&  network_interface_it.get_name().find("en") != string::npos
#endif
                ) {
                // Check if fallback has already been set
                if (network_interface == nullptr) {
                    // Copy the network interface
                    network_interface = unique_ptr<NetworkInterface>(new NetworkInterface(network_interface_it));
                    
                    if (network_interface_it.is_private()) {
                        // We alraedy have the first private network interface
                        break;
                    }
                // Fallback has been set, but this network interface might meet criteria 4
                } else if (network_interface_it.is_private()) {
                    network_interface = unique_ptr<NetworkInterface>(new NetworkInterface(network_interface_it));
                
                    break;
                }
            }
        }

        return network_interface;
    }

    std::vector<std::string> BridgeDiscoveryMethodUtil::list_ips_from_subnets(const std::vector<std::string>& subnets) {
        std::vector<std::string> ips;

        for (auto& subnet : subnets) {
            size_t pos = subnet.find_last_of(".");

            if (pos == string::npos) {
                continue;
            }

            string ip_base = subnet.substr(0, pos + 1);

            for (unsigned int i = bridge_discovery_const::IPSCAN_IP_RANGE_BEGIN; i <= bridge_discovery_const::IPSCAN_IP_RANGE_END; i++) {
                ips.push_back(ip_base + support::to_string(i));
            }
        }

        return ips;
    }

    string BridgeDiscoveryMethodUtil::get_unique_bridge_id_from_mac(const string& mac) {
        // Strip mac address from ':' characters
        string unique_bridge_id(mac);

        unique_bridge_id.erase(std::remove(unique_bridge_id.begin(),  unique_bridge_id.end(), ':'),  unique_bridge_id.end());
        
        if (unique_bridge_id.length() == 12) {
            unique_bridge_id = support::to_upper_case(unique_bridge_id);
            // Build up the mac address with dots
            unique_bridge_id.insert(6, "FFFE");
        }
        
        return unique_bridge_id;
    }

    BridgeDiscoveryIpCheckResult BridgeDiscoveryMethodUtil::parse_bridge_config_result(const std::shared_ptr<BridgeDiscoveryResult>& input, const string& result) {
        BridgeDiscoveryIpCheckResult ip_check_result;
        ip_check_result.ip          = input->get_ip();
        ip_check_result.unique_id   = input->get_unique_id();
        ip_check_result.api_version = input->get_api_version();
        ip_check_result.model_id    = input->get_model_id();
        ip_check_result.reachable   = true;
        
        HUE_LOG << HUE_NETWORK << HUE_DEBUG << "BridgeDiscoveryMethodUtil: parse_bridge_config_result: parsing response -> ip: " << input->get_ip() << ", body: " << result << HUE_ENDL;
        
        if (!result.empty()) {
            if (libjson::is_valid(result)) {
                auto nodes = libjson::parse(result);

                auto bridge_id_node_it = nodes.find("bridgeid");
                auto mac_node_it = nodes.find("mac");
                auto api_version_node_it = nodes.find("apiversion");
                auto model_id_node_it = nodes.find("modelid");
                auto name_node_it = nodes.find("name");
                auto swversion_node_it = nodes.find("swversion");

                if (bridge_id_node_it != nodes.end()) {
                    ip_check_result.unique_id = support::to_upper_case(bridge_id_node_it->as_string());
                } else if (mac_node_it != nodes.end()) {
                    ip_check_result.unique_id = BridgeDiscoveryMethodUtil::get_unique_bridge_id_from_mac(mac_node_it->as_string());
                }

                if (name_node_it != nodes.end()) {
                  ip_check_result.name = name_node_it->as_string();
                }

                if (swversion_node_it != nodes.end()) {
                  ip_check_result.swversion = swversion_node_it->as_string();
                }

                if (!ip_check_result.unique_id.empty()) {
                    ip_check_result.is_bridge = true;
                    
                    HUE_LOG << HUE_CORE << HUE_DEBUG << "BridgeDiscoveryMethodUtil: parse_bridge_config_result: parsed result -> unique id: " << ip_check_result.unique_id << ", ip: " << ip_check_result.ip << HUE_ENDL;

                    if (api_version_node_it != nodes.end()) {
                        ip_check_result.api_version = api_version_node_it->as_string();
                    }

                    if (model_id_node_it != nodes.end()) {
                        ip_check_result.model_id = model_id_node_it->as_string();
                    }
                }
            } else {
                HUE_LOG << HUE_CORE << HUE_ERROR << "BridgeDiscoveryMethodUtil: parse_bridge_config_result: invalid json" << HUE_ENDL;
            }
        }
        
        return ip_check_result;
    }
}  // namespace huesdk
