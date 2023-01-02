/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include "bridgediscovery/BridgeInfo.h"

using std::string;

namespace huesdk {

    const string &BridgeInfo::getIpAddress() const {
        return _ipAddress;
    }

    void BridgeInfo::setIpAddress(const string &ipAddress) {
        _ipAddress = ipAddress;
    }

    const string &BridgeInfo::getId() const {
        return _id;
    }

    void BridgeInfo::setId(const string &id) {
        _id = id;
    }

    const string &BridgeInfo::getUserName() const {
        return _userName;
    }

    void BridgeInfo::setUserName(const string &userName) {
        _userName = userName;
    }

    const bool &BridgeInfo::isAuthorized() const {
        return _authorized;
    }

    void BridgeInfo::setAuthorized(const bool &isAuthorized) {
        _authorized = isAuthorized;
    }

    const bool &BridgeInfo::isIpAddressValid() const {
        return _validIpAddress;
    }

    void BridgeInfo::setIpAddressValid(const bool &isIpAddressValid) {
        _validIpAddress = isIpAddressValid;
    }

    const string &BridgeInfo::getDeviceType() const {
        return _deviceType;
    }

    void BridgeInfo::setDeviceType(const string &deviceType) {
        _deviceType = deviceType;
    }

}  // namespace huesdk
