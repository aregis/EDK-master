/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <map>
#include <string>
#include <chrono>

#include "support/util/NonCopyableBase.h"

namespace huesdk {

    class IHueMDNSService : support::NonCopyableBase {
    public:
        using Host = std::string;
        struct HostInfo {
            std::string ip_address;
            std::chrono::duration<double> discovery_duration;
        };

        virtual std::map<Host, HostInfo> get_hosts() const = 0;
    };

}  // namespace huesdk