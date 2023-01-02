/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <chrono>
#include <memory>
#include <string>
#include <vector>

#include "bridgediscovery/BridgeDiscoveryResult.h"
#include "events/BridgeDiscoveryTaskEventsData.h"
#include "method/mdns/tasks/BridgeDiscoveryMDNSTask.h"
#include "method/mdns/HueMDNSService.h"
#include "tasks/BridgeDiscoveryCheckIpArrayTask.h"

#include "support/logging/Log.h"
#include "support/threading/ConditionVariable.h"
#include "support/threading/Job.h"
#include "support/threading/OperationalQueue.h"
#include "support/threading/QueueExecutor.h"
#include "support/util/Uuid.h"

static constexpr auto SEARCH_INTERVAL = std::chrono::seconds(8);

namespace huesdk {

    template <typename IpCheckTask = huesdk::BridgeDiscoveryCheckIpArrayTask>
    class BridgeDiscoveryMDNSTask : public support::JobTask {
    public:
        using HueMDNSServiceFactory = std::function<std::unique_ptr<IHueMDNSService>(std::shared_ptr<support::OperationalQueue>)>;

        BridgeDiscoveryMDNSTask(
                const boost::uuids::uuid& request_id,
                const std::shared_ptr<IBridgeDiscoveryEventNotifier>& notifier,
                const std::chrono::duration<double>& duration_used_for_search = SEARCH_INTERVAL,
                HueMDNSServiceFactory mdns_service_factory =
                    [](std::shared_ptr<support::OperationalQueue> operational_queue) {return support::make_unique<HueMDNSService>(operational_queue);})
                : _mdns_service_factory{mdns_service_factory},
                  _duration_used_for_search{duration_used_for_search},
                  _stopped_by_user{false},
                  _operation_queue{std::make_shared<support::OperationalQueue>()},
                  _executor{_operation_queue} {
            _task_events_data.request_id = request_id;
            _task_events_data.notifier = notifier;
        }

        void execute(CompletionHandler done) override {
            _task_events_data.start_of_task = std::chrono::system_clock::now();
            std::shared_ptr<IHueMDNSService> mdns_service = _mdns_service_factory(_operation_queue);

            const auto process_results_time_point = std::chrono::steady_clock::now() + std::chrono::duration_cast<std::chrono::milliseconds>(_duration_used_for_search);
            _executor.schedule(process_results_time_point, [this, mdns_service, done] {
                update_events(mdns_service.get());
                if (!_stopped_by_user) {
                    const auto found_ips = get_found_ips(mdns_service.get());
                    schedule_ip_check_tasks(found_ips, done);
                }
            });
        }

        void stop() override {
            _stopped_by_user = true;
        }

        std::vector<std::shared_ptr<BridgeDiscoveryResult>> get_result() const {
            return _results;
        }

    private:
        std::vector<std::string> get_found_ips(IHueMDNSService* mdns_service) const {
            auto found_hosts = mdns_service->get_hosts();

            std::vector<std::string> found_ips;
            for (auto&& host : found_hosts) {
                found_ips.emplace_back(host.second.ip_address);
            }

            return found_ips;
        }

        void update_events(IHueMDNSService* mdns_service) {
            auto found_hosts = mdns_service->get_hosts();

            for (auto&& host : found_hosts) {
                _task_events_data.ip_to_duration_map[host.second.ip_address] = host.second.discovery_duration;
            }
        }

        void schedule_ip_check_tasks(const std::vector<std::string>& found_ips, CompletionHandler done) {
            auto check_ip_array_job = create_job<IpCheckTask>(
                    found_ips, [this](const BridgeDiscoveryIpCheckResult &result) {
                        _results.emplace_back(
                                std::make_shared<BridgeDiscoveryResult>(
                                        result.unique_id, result.ip, result.api_version, result.model_id, result.name, result.swversion));

                        if (_task_events_data.notifier != nullptr) {
                            huesdk::bridge_discovery_events::BridgeDiscovered bridge_discovered_event{
                                    _task_events_data.request_id,
                                    BridgeDiscovery::Option::MDNS,
                                    _task_events_data.ip_to_duration_map[result.ip],
                                    result.ip
                            };

                            _task_events_data.notifier->on_event(bridge_discovered_event);
                        }
                    });
            check_ip_array_job->run([done](IpCheckTask * /*task*/) {
                HUE_LOG << HUE_CORE << HUE_DEBUG
                        << "BridgeDiscoveryMDNSTask: done processing; calling callback" << HUE_ENDL;
                done();
            });
        }

        HueMDNSServiceFactory _mdns_service_factory;
        std::chrono::duration<double> _duration_used_for_search;
        std::vector<std::shared_ptr<BridgeDiscoveryResult>> _results;
        std::atomic<bool> _stopped_by_user;
        BridgeDiscoveryTaskEventsData _task_events_data;
        std::shared_ptr<support::OperationalQueue> _operation_queue = std::make_shared<support::OperationalQueue>();
        support::QueueExecutor _executor;
    };

}  // namespace huesdk