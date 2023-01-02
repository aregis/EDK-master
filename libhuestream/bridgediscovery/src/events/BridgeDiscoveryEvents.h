/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <chrono>
#include <string>
#include <unordered_map>
#include <utility>

#include "bridgediscovery/BridgeDiscovery.h"
#include "support/util/Uuid.h"

namespace huesdk {
    namespace bridge_discovery_events {
        enum class Status {
            SUCCESSFULLY_COMPLETED,
            CANCELLED
        };

        struct DiscoveryStarted {
            boost::uuids::uuid request_id;
        };

        struct DiscoveryFinished {
            boost::uuids::uuid request_id;
            std::chrono::duration<double> duration_in_seconds;
            Status status;
        };

        struct DiscoveryMethodStarted {
            boost::uuids::uuid request_id;
            BridgeDiscovery::Option method_name;
        };

        struct DiscoveryMethodFinished {
            boost::uuids::uuid request_id;
            BridgeDiscovery::Option method_name;
            std::chrono::duration<double> duration_in_seconds;
            Status status;
        };

        struct BridgeDiscovered {
            boost::uuids::uuid request_id;
            BridgeDiscovery::Option method_name;
            std::chrono::duration<double> duration_in_seconds;
            std::string ip;
        };

        struct EventAsString {
            std::string name;
            std::unordered_map<std::string, std::string> data;
        };

        inline bool operator==(const DiscoveryStarted& lhs, const DiscoveryStarted& rhs) {
            return lhs.request_id == rhs.request_id;
        }

        inline bool operator==(const DiscoveryFinished& lhs, const DiscoveryFinished& rhs) {
            return lhs.request_id == rhs.request_id &&
                   lhs.duration_in_seconds == rhs.duration_in_seconds &&
                   lhs.status == rhs.status;
        }

        inline bool operator==(const DiscoveryMethodStarted& lhs, const DiscoveryMethodStarted& rhs) {
            return lhs.request_id == rhs.request_id &&
                   lhs.method_name == rhs.method_name;
        }

        inline bool operator==(const DiscoveryMethodFinished& lhs, const DiscoveryMethodFinished& rhs) {
            return lhs.request_id == rhs.request_id &&
                   lhs.method_name == rhs.method_name &&
                   lhs.duration_in_seconds == rhs.duration_in_seconds &&
                   lhs.status == rhs.status;
        }

        inline bool operator==(const BridgeDiscovered& lhs, const BridgeDiscovered& rhs) {
            return lhs.request_id == rhs.request_id &&
                   lhs.method_name == rhs.method_name &&
                   lhs.duration_in_seconds == rhs.duration_in_seconds &&
                   lhs.ip == rhs.ip;
        }

    }  // namespace bridge_discovery_events

}  // namespace huesdk