/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

namespace huesdk {
    namespace bridge_discovery_events {
        static constexpr auto DISCOVERY_STARTED = "discovery started";
        static constexpr auto DISCOVERY_FINISHED = "discovery finished";
        static constexpr auto DISCOVERY_METHOD_STARTED = "discovery method started";
        static constexpr auto DISCOVERY_METHOD_FINISHED = "discovery method finished";
        static constexpr auto BRIDGE_DISCOVERED = "bridge discovered";

        static constexpr auto DISCOVERY_REQUEST_ID = "request id";
        static constexpr auto DISCOVERY_METHOD = "method";
        static constexpr auto DISCOVERY_DURATION = "duration";
        static constexpr auto DISCOVERED_BRIDGE_IP = "bridge ip";
        static constexpr auto DISCOVERY_STATUS = "status";

        static constexpr auto NUPNP_METHOD = "nupnp";
        static constexpr auto MDNS_METHOD = "mdns";
        static constexpr auto UPNP_METHOD = "upnp";
        static constexpr auto IPSCAN_METHOD = "ipscan";

        static constexpr auto DISCOVERY_STATUS_SUCCESS = "success";
        static constexpr auto DISCOVERY_STATUS_CANCELLED = "cancelled";
    }  // namespace bridge_discovery_events
}  // namespace huesdk