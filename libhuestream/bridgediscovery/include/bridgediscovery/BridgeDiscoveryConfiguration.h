/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <string>
#include <mutex>

#include "bridgediscovery/BridgeDiscoveryReturnCode.h"
#include "BridgeInfo.h"

namespace huesdk {

    class BridgeDiscoveryConfiguration {
    public:
        /**
         Get the bridge discovery URL
         @return The base URL for bridge discovery
         */
        static const char* get_bridge_discovery_url();

        /**
         Get the proxy address
         @return The proxy address to use for http requests
         */
        static std::string get_proxy_address();

        /**
         Get the proxy port number
         @return The proxy port to use for http requests
         */
        static unsigned int get_proxy_port();

        /**
         Set the bridge discovery URL
         @param url The URL for bridge discovery (e.g. "https://discovery.meethue.com")
         */
        static BridgeDiscoveryReturnCode set_bridge_discovery_url(const char* url);
        
        /**
         Enable proxy to use for http requests
         @param address The proxy address to use
         @param port The proxy port to use
         */
        static BridgeDiscoveryReturnCode set_proxy(const char* address, const unsigned int port);
        
        /**
         Whether the proxy is configured for http requests
         @return True when proxy settings were set
         */
        static bool has_proxy_settings();
        
        /**
         Disable the use of a proxy
         */
        static void clear_proxy_settings();

    private:
        static std::string _bridge_discovery_url;

        /** proxy data guard */
        static std::mutex   _proxy_mutex;

        /** if true proxy address and port are valid */
        static bool _has_proxy_settings;

        /** proxy address */
        static std::string _proxy_address;

        /** proxy port */
        static unsigned int _proxy_port;
    };
    
}  // namespace huesdk

