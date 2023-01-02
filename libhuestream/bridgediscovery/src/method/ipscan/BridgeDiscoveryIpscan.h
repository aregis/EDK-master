/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <memory>
#include <string>

#include "bridgediscovery/BridgeDiscoveryClassType.h"
#include "bridgediscovery/IBridgeDiscoveryCallback.h"

#include "events/IBridgeDiscoveryEventNotifier.h"
#include "method/ipscan/tasks/BridgeDiscoveryIpscanTask.h"
#include "method/BridgeDiscoveryMethodBase.h"

namespace huesdk {

    class BridgeDiscoveryIpscan : public BridgeDiscoveryMethodBase<BridgeDiscoveryIpscanTask> {
    public:
        using BridgeDiscoveryMethodBase::MethodResultCallback;

        explicit BridgeDiscoveryIpscan(
                const boost::uuids::uuid& _request_id,
                const std::shared_ptr<IBridgeDiscoveryEventNotifier>& _notifier);

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

