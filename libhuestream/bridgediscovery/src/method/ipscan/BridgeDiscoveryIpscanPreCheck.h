/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace huesdk {

    class BridgeDiscoveryIpscanPreCheck {
    public:
        static std::shared_ptr<BridgeDiscoveryIpscanPreCheck> get_instance();

        static void set_instance(std::shared_ptr<BridgeDiscoveryIpscanPreCheck> instance);

        /**
         Filter ips reachable through port 80
         The function blocks until all ips are probed
         i.e. the callback won't be called after the function returns
         @param ips a list of ipv4 addresses (e.g. ["192.168.1.1", "10.0.2.34"])
         @param callback each reachable ip will be notified through this callback
         */
        virtual std::vector<std::string> filter_reachable_ips(
                std::vector<std::string> ips, const std::atomic<bool> &_stopped_by_user,
                const std::function<void(const std::string&)>& reachable_ip_found);

        virtual ~BridgeDiscoveryIpscanPreCheck() = default;

    private:
        static std::shared_ptr<BridgeDiscoveryIpscanPreCheck> _instance;
    };

}  // namespace huesdk

