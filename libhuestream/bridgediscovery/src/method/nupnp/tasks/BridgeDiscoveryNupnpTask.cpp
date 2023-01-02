/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <memory>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include "libjson/libjson.h"

#include "bridgediscovery/BridgeDiscoveryConfiguration.h"
#include "bridgediscovery/BridgeDiscoveryConst.h"

#include "events/BridgeDiscoveryEvents.h"
#include "method/nupnp/tasks/BridgeDiscoveryNupnpTask.h"
#include "tasks/BridgeDiscoveryCheckIpArrayTask.h"

#include "support/network/http/HttpRequestTask.h"
#include "support/logging/Log.h"

using Task = support::JobTask;
using huesdk::bridge_discovery_events::BridgeDiscovered;
using support::HttpRequestTask;

namespace {
    using huesdk::BridgeDiscoveryResult;
    using huesdk::BridgeDiscoveryConfiguration;

    std::vector<std::shared_ptr<BridgeDiscoveryResult>> parse_nupnp_response(HttpRequestError *error, IHttpResponse *response) {
        std::vector<std::shared_ptr<BridgeDiscoveryResult>> return_value;

        if (error != nullptr && response != nullptr &&
            error->get_code() == HttpRequestError::HTTP_REQUEST_ERROR_CODE_SUCCESS) {
            auto body = response->get_body();

            if (libjson::is_valid(body)) {
                JSONNode nodes = libjson::parse(body);

                for (JSONNode node : nodes) {
                    JSONNode::const_iterator it = node.find("internalipaddress");

                    if (it == node.end()) {
                        HUE_LOG << HUE_CORE << HUE_ERROR
                                << "BridgeDiscoveryNupnpTask: missing internalipaddress node" << HUE_ENDL;
                    } else {
                        return_value.emplace_back(std::make_shared<BridgeDiscoveryResult>("", it->as_string(), "", "", "", ""));
                    }
                }
            } else {
                HUE_LOG << HUE_CORE << HUE_ERROR << "BridgeDiscoveryNupnpTask: invalid json" << HUE_ENDL;
            }
        } else {
            HUE_LOG << HUE_CORE << HUE_ERROR << "BridgeDiscoveryNupnpTask: http request failed" << HUE_ENDL;
        }

        return return_value;
    }

    const HttpRequestTask::Options* http_options_nupnp() {
        static HttpRequestTask::Options options;

        options.connect_timeout = huesdk::bridge_discovery_const::NUPNP_HTTP_CONNECT_TIMEOUT;
        options.receive_timeout = huesdk::bridge_discovery_const::NUPNP_HTTP_RECEIVE_TIMEOUT;
        options.request_timeout = huesdk::bridge_discovery_const::NUPNP_HTTP_REQUEST_TIMEOUT;
        options.use_proxy = BridgeDiscoveryConfiguration::has_proxy_settings();
        options.proxy_address = BridgeDiscoveryConfiguration::get_proxy_address();
        options.proxy_port = static_cast<int>(BridgeDiscoveryConfiguration::get_proxy_port());
        options.verify_ssl = false;  // disabled due to HSDK-2328

        return &options;
    }
}  // namespace

namespace huesdk {
    BridgeDiscoveryNupnpTask::BridgeDiscoveryNupnpTask(
            const std::string& url,
            const boost::uuids::uuid& request_id,
            const std::shared_ptr<IBridgeDiscoveryEventNotifier>& notifier)
       : _url{url} {
        _task_events_data.request_id = request_id;
        _task_events_data.notifier = notifier;
    }

    void BridgeDiscoveryNupnpTask::execute(Task::CompletionHandler done) {
        _task_events_data.start_of_task = {std::chrono::system_clock::now()};
        create_job<HttpRequestTask>(_url, http_options_nupnp())->run([this, done](HttpRequestTask *task) {
            auto results = parse_nupnp_response(task->get_error(), task->get_response());

            for (auto&& result : results) {
                _task_events_data.ip_to_duration_map[result->get_ip()]
                    = std::chrono::system_clock::now() - _task_events_data.start_of_task.value();
            }

            for (auto&& result_entry : results) {
                HUE_LOG << HUE_CORE << HUE_DEBUG << "BridgeDiscovery: submitting IP check for: "
                        << std::string(result_entry->get_unique_id()) << ", ip: " << std::string(result_entry->get_ip())
                        << HUE_ENDL;
            }

            auto check_ip_job = create_job<BridgeDiscoveryCheckIpArrayTask>(results);
            check_ip_job->run([this, done](BridgeDiscoveryCheckIpArrayTask* task) {
                std::vector<std::shared_ptr<BridgeDiscoveryResult>> discovery_ipcheck_results;
                for (const auto &result_entry : task->get_result()) {
                    _results.emplace_back(
                            std::make_shared<BridgeDiscoveryResult>(
                                    result_entry.unique_id, result_entry.ip, result_entry.api_version, result_entry.model_id, result_entry.name, result_entry.swversion));
                }

                if (_task_events_data.notifier != nullptr) {
                    for (auto&& result : _results) {
                        BridgeDiscovered bridge_discovered_event {
                            _task_events_data.request_id,
                            BridgeDiscovery::Option::NUPNP,
                            _task_events_data.ip_to_duration_map[result->get_ip()],
                            result->get_ip()
                        };
                        _task_events_data.notifier->on_event(bridge_discovered_event);
                    }
                }
                done();
            });
        });
    }

    const std::vector<std::shared_ptr<BridgeDiscoveryResult>>& BridgeDiscoveryNupnpTask::get_result() const {
        return _results;
    }
}  // namespace huesdk
