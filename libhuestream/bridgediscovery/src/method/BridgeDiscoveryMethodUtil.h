/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <memory>
#include <string>
#include <atomic>
#include <vector>
#include <future>

#include "support/network/NetworkInterface.h"

#include "bridgediscovery/BridgeInfo.h"
#include "bridgediscovery/BridgeDiscoveryIpCheckResult.h"

#include "method/IBridgeDiscoveryMethod.h"

namespace huesdk {
    // forward declaration
    class BridgeConfiguration;
    
    class BridgeDiscoveryMethodUtil {
    public:
        /*
         Get network interface by the following criteria
         1) Interface which has a IPV4 ip address
         2) Interface which is not a loopback
         3) Interface which is up
         4) Interface has an IP which is RFC1918 compliant
         
         OSX only:
         5) Interface should have "en" name, which excluded mobile interfaces
         
         If no network has been found, it will skip the 4th critera and return the first
         interface which matches the first three
        */
        static std::unique_ptr<support::NetworkInterface> get_first_private_network_interface();
        
        /**
         List all ips in a vector of ipv4 subnets
         @param subnets The subnets, e.g. ["192.168.1.0", "10.0.2.0"]
         @return Vector of ips, e.g. ["192.168.1.1", "192.168.1.2", ..., "10.0.2.254"]
        */
        static std::vector<std::string> list_ips_from_subnets(const std::vector<std::string>& subnets);

        /**
         Get unique bridge id from a given mac address (with or without ':')
         Example 01:23:45:67:89:ab -> 012345FFFE6789AB
         */
        static std::string get_unique_bridge_id_from_mac(const std::string& mac);

        /**
         Parse the response of the http request for the bridge config JSON
         @param  ip     The ip address which has been scanned
         @param  result The http response body
         @return struct with the information
         */
        static BridgeDiscoveryIpCheckResult parse_bridge_config_result(const std::shared_ptr<BridgeDiscoveryResult>& input, const std::string& result);
    };

}  // namespace huesdk

