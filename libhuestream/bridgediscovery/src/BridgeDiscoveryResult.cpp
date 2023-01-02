/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include "bridgediscovery/BridgeDiscoveryResult.h"

#include <string>

using std::string;

namespace huesdk {

    BridgeDiscoveryResult::BridgeDiscoveryResult(const string &unique_id, const string &ip, const string &api_version, const string &model_id, const std::string& name, const std::string& swversion)
            : _unique_id(unique_id),
              _ip(ip),
              _api_version(api_version),
              _model_id(model_id),
              _name(name),
              _swversion(swversion) {
    }

    BridgeDiscoveryResult::BridgeDiscoveryResult(const string &ip)
            : _ip(ip) {
    }

    const char *BridgeDiscoveryResult::get_unique_id() const {
        return _unique_id.c_str();
    }

    const char *BridgeDiscoveryResult::get_ip() const {
        return _ip.c_str();
    }

    const char *BridgeDiscoveryResult::get_api_version() const {
        return _api_version.c_str();
    }

    const char *BridgeDiscoveryResult::get_model_id() const {
        return _model_id.c_str();
    }

    const char* BridgeDiscoveryResult::get_name() const {
      return _name.c_str();
    }

    const char* BridgeDiscoveryResult::get_swversion() const {
      return _swversion.c_str();
    }

    void BridgeDiscoveryResult::set_unique_id(std::string s) {
        _unique_id = std::move(s);
    }

    void BridgeDiscoveryResult::set_ip(std::string s) {
        _ip = std::move(s);
    }

    void BridgeDiscoveryResult::set_api_version(std::string s) {
        _api_version = std::move(s);
    }

    void BridgeDiscoveryResult::set_model_id(std::string s) {
        _model_id = std::move(s);
    }

}  // namespace huesdk
