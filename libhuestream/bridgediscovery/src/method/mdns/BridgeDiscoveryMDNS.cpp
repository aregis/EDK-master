/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include "method/mdns/BridgeDiscoveryMDNS.h"

namespace huesdk {

    BridgeDiscoveryMDNS::BridgeDiscoveryMDNS(
            const boost::uuids::uuid& request_id,
            const std::shared_ptr<IBridgeDiscoveryEventNotifier>& notifier)
            : BridgeDiscoveryMethodBase<BridgeDiscoveryMDNSTask<>>(request_id, notifier) {}

    bool BridgeDiscoveryMDNS::method_search(const MethodResultCallback &callback) {
        _job = support::create_job<TaskType>(_request_id, _bridge_discovery_event_notifier);

        return _job->run([callback](TaskType *task) {
            callback(task->get_result());
        });
    }

    BridgeDiscoveryClassType BridgeDiscoveryMDNS::get_type() const {
        return BRIDGE_DISCOVERY_CLASS_TYPE_MDNS;
    }

}  // namespace huesdk