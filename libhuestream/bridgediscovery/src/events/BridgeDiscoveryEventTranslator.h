/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <chrono>
#include <ios>
#include <sstream>
#include <string>
#include <utility>

#include "boost/lexical_cast.hpp"

#include "BridgeDiscoveryEventNames.h"
#include "BridgeDiscoveryEvents.h"
#include "support/logging/Log.h"
#include "support/util/LexicalCastForBool.h"
#include "support/util/Uuid.h"

namespace huesdk {
    namespace bridge_discovery_events {

        class BridgeDiscoveryEventTranslator {
        public:
            static bridge_discovery_events::EventAsString translate(const bridge_discovery_events::DiscoveryStarted& e) {
                bridge_discovery_events::EventAsString result {};
                result.name = bridge_discovery_events::DISCOVERY_STARTED;
                result.data[bridge_discovery_events::DISCOVERY_REQUEST_ID] = boost::lexical_cast<std::string>(e.request_id);
                return result;
            }

            static bridge_discovery_events::EventAsString translate(const bridge_discovery_events::DiscoveryFinished& e) {
                bridge_discovery_events::EventAsString result {};
                result.name = bridge_discovery_events::DISCOVERY_FINISHED;
                result.data[bridge_discovery_events::DISCOVERY_REQUEST_ID] = boost::lexical_cast<std::string>(e.request_id);
                result.data[bridge_discovery_events::DISCOVERY_DURATION]
                        = boost::lexical_cast<std::string>(
                                std::chrono::duration_cast<std::chrono::milliseconds>(e.duration_in_seconds).count());
                result.data[bridge_discovery_events::DISCOVERY_STATUS] = to_string(e.status);

                return result;
            }

            static bridge_discovery_events::EventAsString translate(const bridge_discovery_events::DiscoveryMethodStarted& e) {
                bridge_discovery_events::EventAsString result {};
                result.name = bridge_discovery_events::DISCOVERY_METHOD_STARTED;
                result.data[bridge_discovery_events::DISCOVERY_REQUEST_ID] = boost::lexical_cast<std::string>(e.request_id);
                result.data[bridge_discovery_events::DISCOVERY_METHOD] = to_string(e.method_name);
                return result;
            }

            static bridge_discovery_events::EventAsString translate(const bridge_discovery_events::DiscoveryMethodFinished& e) {
                bridge_discovery_events::EventAsString result {};
                result.name = bridge_discovery_events::DISCOVERY_METHOD_FINISHED;
                result.data[bridge_discovery_events::DISCOVERY_REQUEST_ID] = boost::lexical_cast<std::string>(e.request_id);
                result.data[bridge_discovery_events::DISCOVERY_METHOD] = to_string(e.method_name);
                result.data[bridge_discovery_events::DISCOVERY_DURATION] = boost::lexical_cast<std::string>(
                        std::chrono::duration_cast<std::chrono::milliseconds>(e.duration_in_seconds).count());
                result.data[bridge_discovery_events::DISCOVERY_STATUS] = to_string(e.status);

                return result;
            }

            static bridge_discovery_events::EventAsString translate(const bridge_discovery_events::BridgeDiscovered& e) {
                bridge_discovery_events::EventAsString result {};
                result.name = bridge_discovery_events::BRIDGE_DISCOVERED;
                result.data[bridge_discovery_events::DISCOVERY_REQUEST_ID] = boost::lexical_cast<std::string>(e.request_id);
                result.data[bridge_discovery_events::DISCOVERED_BRIDGE_IP] = e.ip;
                result.data[bridge_discovery_events::DISCOVERY_METHOD] = to_string(e.method_name);
                result.data[bridge_discovery_events::DISCOVERY_DURATION]
                        = boost::lexical_cast<std::string>(
                            std::chrono::duration_cast<std::chrono::milliseconds>(e.duration_in_seconds).count());

                return result;
            }

        private:
            static std::string to_string(const BridgeDiscovery::Option& o) {
                switch (o) {
                    case BridgeDiscovery::Option::UPNP:
                        return bridge_discovery_events::UPNP_METHOD;
                    case BridgeDiscovery::Option::IPSCAN:
                        return bridge_discovery_events::IPSCAN_METHOD;
                    case BridgeDiscovery::Option::NUPNP:
                        return bridge_discovery_events::NUPNP_METHOD;
                    case BridgeDiscovery::Option::MDNS:
                        return bridge_discovery_events::MDNS_METHOD;
                    default:
                        HUE_LOG << HUE_CORE << HUE_ERROR
                            << "Can't convert bridge discovery option to string" << HUE_ENDL;
                }
                return {};
            }

            static std::string to_string(const bridge_discovery_events::Status& s) {
                switch (s) {
                    case bridge_discovery_events::Status::SUCCESSFULLY_COMPLETED:
                        return bridge_discovery_events::DISCOVERY_STATUS_SUCCESS;
                    case bridge_discovery_events::Status::CANCELLED:
                        return bridge_discovery_events::DISCOVERY_STATUS_CANCELLED;
                    default:
                        HUE_LOG << HUE_CORE << HUE_ERROR
                            << "Can't convert bridge discovery status to string" << HUE_ENDL;
                }
                return {};
            }
        };

    }  // namespace bridge_discovery_events
}  // namespace huesdk
