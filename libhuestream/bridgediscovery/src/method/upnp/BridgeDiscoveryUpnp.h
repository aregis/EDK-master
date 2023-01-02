/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <memory>
#include <string>

#include "events/IBridgeDiscoveryEventNotifier.h"
#include "method/upnp/tasks/BridgeDiscoveryUpnpTask.h"
#include "method/BridgeDiscoveryMethodBase.h"
#include "support/util/Uuid.h"

namespace huesdk {

    class BridgeDiscoveryUpnp : public BridgeDiscoveryMethodBase<BridgeDiscoveryUpnpTask> {
    public:
        using BridgeDiscoveryMethodBase::MethodResultCallback;

        explicit BridgeDiscoveryUpnp(
                const boost::uuids::uuid& request_id,
                const std::shared_ptr<IBridgeDiscoveryEventNotifier>& notifier);

        /**
        @see IBridgeDiscoveryMethod.h
        */
        BridgeDiscoveryClassType get_type() const override;

    protected:
        /**
        @see BridgeDiscoveryMethodBase.h
        */
        bool method_search(const MethodResultCallback &callback) override;
    };

}  // namespace huesdk

