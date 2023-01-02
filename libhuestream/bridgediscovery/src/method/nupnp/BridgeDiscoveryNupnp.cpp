/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include "support/logging/Log.h"

#include "bridgediscovery/BridgeDiscoveryConst.h"
#include "bridgediscovery/BridgeDiscoveryConfiguration.h"

#include "method/nupnp/BridgeDiscoveryNupnp.h"
#include "tasks/BridgeDiscoveryNupnpTask.h"

using support::JobState;

namespace huesdk {
    BridgeDiscoveryNupnp::BridgeDiscoveryNupnp(
            const boost::uuids::uuid& request_id,
            const std::shared_ptr<IBridgeDiscoveryEventNotifier>& notifier)
            : BridgeDiscoveryMethodBase<BridgeDiscoveryNupnpTask>(request_id, notifier) {}

    bool BridgeDiscoveryNupnp::method_search(const MethodResultCallback &callback) {
        _job = support::create_job<TaskType>(
                get_bridge_discovery_nupnp_url(),
                _request_id,
                _bridge_discovery_event_notifier);

        return _job->run([callback](TaskType *task) {
            callback(task->get_result());
        });
    }

    BridgeDiscoveryClassType BridgeDiscoveryNupnp::get_type() const {
        return BridgeDiscoveryClassType::BRIDGE_DISCOVERY_CLASS_TYPE_NUPNP;
    }

    std::string BridgeDiscoveryNupnp::get_bridge_discovery_nupnp_url() {
        string url = BridgeDiscoveryConfiguration::get_bridge_discovery_url();

        // add the /api/nupnp suffix to the legacy urls for backward compatibility

        if (url == "https://www.meethue.com" ||
            url == "https://www-system.meethue.com" ||
            url == "https://www-portal.meethue.com") {
            return url + bridge_discovery_const::NUPNP_HTTP_URL_PATH;
        }

        // any other url is used as is

        return url;
    }
}  // namespace huesdk
