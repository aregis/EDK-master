/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <string>

#include "support/logging/Log.h"

#include "bridgediscovery/BridgeDiscoveryConfiguration.h"

#include "method/BridgeDiscoveryMethodUtil.h"

#ifdef _WIN32
    #define snprintf _snprintf
#endif

using std::string;

namespace huesdk {

    string BridgeDiscoveryConfiguration::_bridge_discovery_url = "https://discovery.meethue.com";

    std::mutex   BridgeDiscoveryConfiguration::_proxy_mutex;
    bool BridgeDiscoveryConfiguration::_has_proxy_settings = false;
    string BridgeDiscoveryConfiguration::_proxy_address = "";
    unsigned int BridgeDiscoveryConfiguration::_proxy_port = 0;

    const char* BridgeDiscoveryConfiguration::get_bridge_discovery_url() {
        return _bridge_discovery_url.c_str();
    }
    
    std::string BridgeDiscoveryConfiguration::get_proxy_address() {
        std::lock_guard<mutex> proxy_lock(_proxy_mutex);

        return std::string(_proxy_address);
    }
    
    unsigned int BridgeDiscoveryConfiguration::get_proxy_port() {
        std::lock_guard<mutex> proxy_lock(_proxy_mutex);
        
        return _proxy_port;
    }
    
    BridgeDiscoveryReturnCode BridgeDiscoveryConfiguration::set_bridge_discovery_url(const char* url) {
        if (url == nullptr) {
            HUE_LOG << HUE_CORE << HUE_ERROR << "Sdk: bridge discovery url is null" << HUE_ENDL;
            
            return BRIDGE_DISCOVERY_RETURN_CODE_NULL_PARAMETER;
        }
        
        _bridge_discovery_url = url;
        
        return BRIDGE_DISCOVERY_RETURN_CODE_SUCCESS;
    }
    
    BridgeDiscoveryReturnCode BridgeDiscoveryConfiguration::set_proxy(const char* address, const unsigned int port) {
        std::lock_guard<mutex> proxy_lock(_proxy_mutex);
        
        if (address == nullptr) {
            HUE_LOG << HUE_CORE << HUE_ERROR << "Sdk: proxy address is null" << HUE_ENDL;
            
            return BRIDGE_DISCOVERY_RETURN_CODE_NULL_PARAMETER;
        }
        
        _proxy_address = address;
        _proxy_port = port;
        _has_proxy_settings = true;
        
        return BRIDGE_DISCOVERY_RETURN_CODE_SUCCESS;
    }
    
    bool BridgeDiscoveryConfiguration::has_proxy_settings() {
        std::lock_guard<mutex> proxy_lock(_proxy_mutex);
        
        return _has_proxy_settings;
    }
    
    void BridgeDiscoveryConfiguration::clear_proxy_settings() {
        std::lock_guard<mutex> proxy_lock(_proxy_mutex);
        
        _proxy_address = "";
        _proxy_port = 0;
        _has_proxy_settings = false;
    }

}  // namespace huesdk
