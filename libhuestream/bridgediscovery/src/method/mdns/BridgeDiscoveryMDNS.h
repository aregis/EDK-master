/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <memory>

#include "boost/uuid/uuid.hpp"

#include "bridgediscovery/BridgeDiscoveryClassType.h"
#include "events/IBridgeDiscoveryEventNotifier.h"
#include "method/mdns/tasks/BridgeDiscoveryMDNSTask.h"
#include "method/BridgeDiscoveryMethodBase.h"

namespace huesdk {

    class BridgeDiscoveryMDNS : public BridgeDiscoveryMethodBase<BridgeDiscoveryMDNSTask<>> {
    public:
        using BridgeDiscoveryMethodBase::MethodResultCallback;

        BridgeDiscoveryMDNS(
                const boost::uuids::uuid& _request_id, const std::shared_ptr<IBridgeDiscoveryEventNotifier>& _notifier);

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
