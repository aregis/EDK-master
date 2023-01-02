/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <string>
#include <vector>
#include <memory>
#include <mutex>

#include "events/IBridgeDiscoveryEventNotifier.h"
#include "method/nupnp/tasks/BridgeDiscoveryNupnpTask.h"
#include "method/BridgeDiscoveryMethodBase.h"
#include "support/util/Uuid.h"

namespace huesdk {
    class BridgeDiscoveryNupnp : public BridgeDiscoveryMethodBase<BridgeDiscoveryNupnpTask> {
    public:
        using BridgeDiscoveryMethodBase::MethodResultCallback;

        explicit BridgeDiscoveryNupnp(
                const boost::uuids::uuid& request_id,
                const std::shared_ptr<IBridgeDiscoveryEventNotifier>& notifier);

        /**
        @see IBridgeDiscoveryMethod.h
        */
        BridgeDiscoveryClassType get_type() const override;

        static std::string get_bridge_discovery_nupnp_url();

    protected:
        /**
        @see BridgeDiscoveryMethodBase.h
        */
        bool method_search(const MethodResultCallback &callback) override;
    };

}  // namespace huesdk

