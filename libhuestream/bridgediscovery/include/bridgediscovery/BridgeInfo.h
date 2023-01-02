/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <string>
#include <memory>
#include <vector>

namespace huesdk {

    class BridgeInfo {
    public:
        BridgeInfo() :
                _ipAddress(),
                _id(),
                _deviceType(),
                _validIpAddress(false),
                _authorized(false) { }

        BridgeInfo(const std::string &id, const std::string &ipAddress, bool validIp) :
                _ipAddress(ipAddress),
                _id(id),
                _deviceType(),
                _validIpAddress(validIp),
                _authorized(false) { }

        const std::string &getIpAddress() const;

        void setIpAddress(const std::string &ipAddress);

        const std::string &getId() const;

        void setId(const std::string &id);

        const std::string &getUserName() const;

        void setUserName(const std::string &userName);

        const std::string &getDeviceType() const;

        void setDeviceType(const std::string &deviceType);

        const bool &isAuthorized() const;

        void setAuthorized(const bool &isAuthorized);

        const bool &isIpAddressValid() const;

        void setIpAddressValid(const bool &isIpAddressValid);


    protected:
        std::string _ipAddress;
        std::string _id;
        std::string _userName;
        std::string _deviceType;
        bool        _validIpAddress;
        bool        _authorized;
    };

}  // namespace huesdk

