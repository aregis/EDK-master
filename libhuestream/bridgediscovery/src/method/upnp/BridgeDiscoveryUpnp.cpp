/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <string>

#include "support/logging/Log.h"

#include "bridgediscovery/BridgeDiscovery.h"
#include "bridgediscovery/BridgeDiscoveryConst.h"

#include "method/BridgeDiscoveryMethodUtil.h"
#include "method/upnp/BridgeDiscoveryUpnp.h"

using support::JobState;

namespace huesdk {
    BridgeDiscoveryUpnp::BridgeDiscoveryUpnp(
            const boost::uuids::uuid& request_id,
            const std::shared_ptr<IBridgeDiscoveryEventNotifier>& notifier)
            : BridgeDiscoveryMethodBase<BridgeDiscoveryUpnpTask>(request_id, notifier) {}

    bool BridgeDiscoveryUpnp::method_search(const BridgeDiscoveryMethodBase::MethodResultCallback &callback) {
        _job = support::create_job<TaskType>(_request_id, _bridge_discovery_event_notifier);

        return _job->run([callback](TaskType *task) {
            callback(task->get_result());
        });
    }

    BridgeDiscoveryClassType BridgeDiscoveryUpnp::get_type() const {
        return BRIDGE_DISCOVERY_CLASS_TYPE_UPNP;
    }
}  // namespace huesdk
