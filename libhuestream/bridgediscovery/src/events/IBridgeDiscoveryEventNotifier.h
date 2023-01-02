/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include "events/BridgeDiscoveryEvents.h"

namespace huesdk {

    class IBridgeDiscoveryEventNotifier {
    public:
        virtual ~IBridgeDiscoveryEventNotifier() = default;
        IBridgeDiscoveryEventNotifier() = default;

        IBridgeDiscoveryEventNotifier(const IBridgeDiscoveryEventNotifier&) = delete;
        IBridgeDiscoveryEventNotifier(IBridgeDiscoveryEventNotifier&&) = delete;

        virtual void on_event(const bridge_discovery_events::DiscoveryStarted&) const = 0;
        virtual void on_event(const bridge_discovery_events::DiscoveryMethodStarted&) const = 0;
        virtual void on_event(const bridge_discovery_events::BridgeDiscovered&) const = 0;
        virtual void on_event(const bridge_discovery_events::DiscoveryMethodFinished&) const = 0;
        virtual void on_event(const bridge_discovery_events::DiscoveryFinished&) const = 0;
    };

}  // namespace huesdk