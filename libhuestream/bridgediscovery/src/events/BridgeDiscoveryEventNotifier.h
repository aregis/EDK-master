/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <memory>

#include "events/BridgeDiscoveryEventTranslator.h"
#include "events/IBridgeDiscoveryEventNotifier.h"
#include "support/threading/QueueDispatcher.h"
#include "support/util/EventNotifier.h"

namespace huesdk {

    using support::EventNotifier;

    class BridgeDiscoveryEventNotifier final : public IBridgeDiscoveryEventNotifier {
    public:
        explicit BridgeDiscoveryEventNotifier(const EventNotifier& notifier)
          : _notifier(notifier) {}

        void on_event(const bridge_discovery_events::DiscoveryStarted& e) const override {
            on_event_impl(e);
        }

        void on_event(const bridge_discovery_events::DiscoveryMethodStarted& e) const override {
            on_event_impl(e);
        }

        void on_event(const bridge_discovery_events::BridgeDiscovered& e) const override {
            on_event_impl(e);
        }

        void on_event(const bridge_discovery_events::DiscoveryMethodFinished& e) const override {
            on_event_impl(e);
        }

        void on_event(const bridge_discovery_events::DiscoveryFinished& e) const override {
            on_event_impl(e);
        }

    private:
        support::EventNotifier _notifier;
        mutable support::QueueDispatcher _dispatcher;

        template <typename EventType>
        void on_event_impl(const EventType& e) const {
            _dispatcher.post([this, e]() {
                auto str = bridge_discovery_events::BridgeDiscoveryEventTranslator::translate(e);
                _notifier(str.name, str.data);
            });
        }
    };

}  // namespace huesdk