/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <algorithm>
#include <string>
#include <memory>
#include <vector>

#include "support/logging/Log.h"

#include "method/ipscan/tasks/BridgeDiscoveryIpscanTask.h"
#include "method/ipscan/BridgeDiscoveryIpscanPreCheck.h"
#include "tasks/BridgeDiscoveryCheckIpArrayTask.h"

using Task = support::JobTask;
using support::OperationalQueue;
using support::NetworkInterface;
using support::QueueExecutor;

using huesdk::bridge_discovery_events::BridgeDiscovered;

namespace {
    using huesdk::BridgeDiscoveryMethodUtil;

    std::vector<std::string> get_ips_to_check() {
        auto network_interface = BridgeDiscoveryMethodUtil::get_first_private_network_interface();

        if (network_interface == nullptr) {
            HUE_LOG << HUE_CORE << HUE_ERROR << "BridgeDiscoveryIpscan: no valid network interface available"
                    << HUE_ENDL;
            return {};
        } else {
            HUE_LOG << HUE_CORE << HUE_DEBUG << "BridgeDiscoveryIpscan: network interface found -> name: "
                    << network_interface->get_name() + ", ip: " << network_interface->get_ip() << HUE_ENDL;
            return BridgeDiscoveryMethodUtil::list_ips_from_subnets({network_interface->get_ip()});
        }
    }
}  // namespace

namespace huesdk {
    BridgeDiscoveryIpscanTask::BridgeDiscoveryIpscanTask(
            const boost::uuids::uuid& request_id,
            const std::shared_ptr<IBridgeDiscoveryEventNotifier>& notifier)
       : _executor(std::make_shared<OperationalQueue>()), _stopped_by_user(false) {
        _task_events_data.request_id = request_id;
        _task_events_data.notifier = notifier;
    }

    void BridgeDiscoveryIpscanTask::execute(Task::CompletionHandler done) {
        _task_events_data.start_of_task = {std::chrono::system_clock::now()};

        _executor.execute([this, done]() {
            auto ip_found_cb = [this](const std::string& ip) {
                _task_events_data.ip_to_duration_map[ip] = std::chrono::system_clock::now() - _task_events_data.start_of_task.value();
            };

            auto filtered_ips = BridgeDiscoveryIpscanPreCheck::get_instance()->filter_reachable_ips(
                    get_ips_to_check(), _stopped_by_user, ip_found_cb);

            auto check_ip_array_job = create_job<BridgeDiscoveryCheckIpArrayTask>(filtered_ips, [this](const BridgeDiscoveryIpCheckResult &result) {
                _results.emplace_back(std::make_shared<BridgeDiscoveryResult>(result.unique_id, result.ip, result.api_version, result.model_id, result.name, result.swversion));

                if (_task_events_data.notifier != nullptr) {
                    BridgeDiscovered bridge_discovered_event{
                            _task_events_data.request_id,
                            BridgeDiscovery::Option::IPSCAN,
                            _task_events_data.ip_to_duration_map[result.ip],
                            result.ip
                    };

                    _task_events_data.notifier->on_event(bridge_discovered_event);
                }
            });
            check_ip_array_job->run([done](BridgeDiscoveryCheckIpArrayTask *) {
                done();
            });
        });
    }

    const std::vector<std::shared_ptr<BridgeDiscoveryResult>>& BridgeDiscoveryIpscanTask::get_result() const {
        return _results;
    }

    void BridgeDiscoveryIpscanTask::stop() {
        _stopped_by_user = true;
    }

}  // namespace huesdk
