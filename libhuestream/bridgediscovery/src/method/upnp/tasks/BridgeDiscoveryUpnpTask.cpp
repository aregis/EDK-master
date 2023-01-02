/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <memory>
#include <regex>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include "bridgediscovery/BridgeDiscoveryConst.h"
#include "BridgeDiscoveryUpnpListenTask.h"
#include "BridgeDiscoveryUpnpSendTask.h"
#include "events/BridgeDiscoveryEvents.h"
#include "events/IBridgeDiscoveryEventNotifier.h"
#include "method/upnp/tasks/BridgeDiscoveryUpnpTask.h"
#include "tasks/BridgeDiscoveryCheckIpArrayTask.h"

#include "support/network/sockets/SocketAddress.h"
#include "support/network/sockets/SocketError.h"
#include "support/logging/Log.h"

using std::regex;
using std::regex_search;
using std::smatch;
using std::vector;

using huesdk::bridge_discovery_events::BridgeDiscovered;

using support::JobTask;
using support::JobState;
using support::NestedJob;
using support::SocketError;
using support::SocketAddress;
using support::to_string;
using support::Timer;
using support::NetworkInterface;

namespace {
    /** consts for the matched regex results */
    const int IP_POS = 1;
    const int API_VERSION_POS = 2;
    const int MAC_POS = 3;
    const int BRIDGE_ID_POS = 5;
    
    unique_ptr<SOCKET_UDP> init_socket() {
        unique_ptr<SOCKET_UDP> socket;

        HUE_LOG << HUE_CORE << HUE_DEBUG << "BridgeDiscoveryUpnp: initialize socket" << HUE_ENDL;

        {  // Initialisation
            HUE_LOG << HUE_CORE << HUE_DEBUG << "BridgeDiscoveryUpnp: get network interface" << HUE_ENDL;

            unique_ptr<NetworkInterface> network_interface = huesdk::BridgeDiscoveryMethodUtil::get_first_private_network_interface();

            if (network_interface != nullptr) {
                HUE_LOG << HUE_CORE << HUE_DEBUG << "BridgeDiscoveryUpnp: binding to ip: "
                        << network_interface->get_ip() << HUE_ENDL;

                // Create and open new socket
                unique_ptr<SOCKET_UDP> socket_init = unique_ptr<SOCKET_UDP>(new SOCKET_UDP(SocketAddress(network_interface->get_ip(), 0)));
                // Configure the socket
                socket_init->set_reuse_address(true);
                socket_init->set_broadcast(true);
                socket_init->join_group(SocketAddress(huesdk::bridge_discovery_const::UDP_SOCKET_IP, 0));
                // Bind the socket to the local address
                int socket_status = socket_init->bind();

                HUE_LOG << HUE_CORE << HUE_DEBUG << "BridgeDiscoveryUpnp: initialize socket -> join group: "
                        << huesdk::bridge_discovery_const::UDP_SOCKET_IP << HUE_ENDL;

                if (socket_status == support::SOCKET_STATUS_OK) {
                    // Assign the initialized socket
                    socket = move(socket_init);
                }
            } else {
                HUE_LOG << HUE_CORE << HUE_ERROR << "BridgeDiscoveryUpnp: no valid network interface available"
                        << HUE_ENDL;
            }
        }

        HUE_LOG << HUE_CORE << HUE_DEBUG << "BridgeDiscoveryUpnp: socket initialized" << HUE_ENDL;

        return socket;
    }

    struct IntermediateParseResult {
        std::vector<std::shared_ptr<huesdk::BridgeDiscoveryResult>> processed_results;
        std::string remaining_unparsed_data;
    };

    IntermediateParseResult parse_response(std::string response) {
        std::vector<std::shared_ptr<huesdk::BridgeDiscoveryResult>> results;

        smatch result;
        // Parse the found bridges from the data string
        while (regex_search(response, result, huesdk::bridge_discovery_const::UPNP_RESPONSE_REGEX)) {
            string ip = string(result[IP_POS].first, result[IP_POS].second);
            string api_version = string(result[API_VERSION_POS].first, result[API_VERSION_POS].second);
            string unique_bridge_id;

            if (result[BRIDGE_ID_POS].matched) {
                unique_bridge_id = support::to_upper_case(string(result[BRIDGE_ID_POS].first, result[BRIDGE_ID_POS].second));
            } else {
                string mac = string(result[MAC_POS].first, result[MAC_POS].second);
                unique_bridge_id = huesdk::BridgeDiscoveryMethodUtil::get_unique_bridge_id_from_mac(mac);
            }

            results.emplace_back(std::make_shared<huesdk::BridgeDiscoveryResult>(unique_bridge_id, ip, api_version, "", "", ""));

            // Gets sequence after last submatch.
            response = result.suffix().str();
        }

        // Parse the found bridges, with firmware > 1.9
        while (regex_search(response, result, huesdk::bridge_discovery_const::UPNP_RESPONSE_REGEX_19)) {
            string ip = string(result[IP_POS].first, result[IP_POS].second);
            string api_version = string(result[API_VERSION_POS].first, result[API_VERSION_POS].second);
            string unique_bridge_id = string(result[MAC_POS].first, result[MAC_POS].second);

            results.emplace_back(std::make_shared<huesdk::BridgeDiscoveryResult>(unique_bridge_id, ip, api_version, "", "", ""));

            // Gets sequence after last submatch.
            response = result.suffix().str();
        }

        return {results, response};
    }
}  // namespace

namespace huesdk {
    BridgeDiscoveryUpnpTask::BridgeDiscoveryUpnpTask(
            const boost::uuids::uuid& request_id,
            const std::shared_ptr<IBridgeDiscoveryEventNotifier>& notifier)
       : _socket(init_socket()) {
        _task_events_data.request_id = request_id;
        _task_events_data.notifier = notifier;
    }

    void BridgeDiscoveryUpnpTask::execute(JobTask::CompletionHandler done) {
        _done = std::move(done);
        _task_events_data.start_of_task = {std::chrono::system_clock::now()};

        if (_socket == nullptr) {
            _done();
        } else {
            _timeout_timer.reset(new Timer(bridge_discovery_const::UDP_TIMEOUT, [this]() {
                HUE_LOG << HUE_CORE << HUE_DEBUG << "BridgeDiscoveryUpnp: timeout timer expired, so stop searching"
                        << HUE_ENDL;
                _socket->close();
            }));

            _send_job = support::create_job<BridgeDiscoveryUpnpSendTask>(_socket.get());
            _send_job->run([this](BridgeDiscoveryUpnpSendTask *task) {
                if (task->is_successful()) {
                    start_listen_task();
                } else {
                    _done();
                }
            });

            _timeout_timer->start();
        }
    }

    void BridgeDiscoveryUpnpTask::stop() {
        _timeout_timer->stop();
        _socket->close();

        if (_send_job != nullptr && _send_job->state() == JobState::Running) {
            _send_job->cancel();
        }
    }

    const vector<std::shared_ptr<BridgeDiscoveryResult>> &BridgeDiscoveryUpnpTask::get_result() const {
        return _filtered_results;
    }

    void BridgeDiscoveryUpnpTask::start_listen_task() {
        auto listen_job = create_job<BridgeDiscoveryUpnpListenTask>(_socket.get());
        listen_job->run([this](BridgeDiscoveryUpnpListenTask* task) {
            if (_socket->is_opened()) {
                auto task_result = task->get_result();
                if (!task_result.empty()) {
                    _unparsed_listen_result += std::move(task_result);

                    HUE_LOG << HUE_CORE << HUE_DEBUG
                            << "BridgeDiscoveryUpnp: parse partial results " << _unparsed_listen_result << HUE_ENDL;

                    auto intermediate_parse_result = parse_response(_unparsed_listen_result);

                    _unfiltered_results.insert(
                            _unfiltered_results.end(),
                            intermediate_parse_result.processed_results.begin(),
                            intermediate_parse_result.processed_results.end());

                    _unparsed_listen_result = intermediate_parse_result.remaining_unparsed_data;

                    for (auto&& result : intermediate_parse_result.processed_results) {
                        if (result != nullptr) {
                            _task_events_data.ip_to_duration_map[result->get_ip()]
                               = std::chrono::system_clock::now() - _task_events_data.start_of_task.value();
                        }
                    }
                }

                start_listen_task();
            } else {
                for (auto&& result_entry : _unfiltered_results) {
                    HUE_LOG << HUE_CORE << HUE_DEBUG << "BridgeDiscovery: submitting IP check for: "
                            << std::string(result_entry->get_unique_id()) << ", ip: " << std::string(result_entry->get_ip())
                            << HUE_ENDL;
                }

                auto check_ip_job = create_job<BridgeDiscoveryCheckIpArrayTask>(_unfiltered_results);

                check_ip_job->run([this](BridgeDiscoveryCheckIpArrayTask* task) {
                    std::vector<std::shared_ptr<BridgeDiscoveryResult>> discovery_ipcheck_results;
                    for (auto&& result_entry : task->get_result()) {
                        _filtered_results.emplace_back(
                                std::make_shared<BridgeDiscoveryResult>(
                                        result_entry.unique_id, result_entry.ip,
                                        result_entry.api_version, result_entry.model_id,
                                        result_entry.name, result_entry.swversion));
                    }

                    if (_task_events_data.notifier != nullptr) {
                        for (auto&& result : _filtered_results) {
                            BridgeDiscovered bridge_discovered_event {
                                    _task_events_data.request_id,
                                    BridgeDiscovery::Option::UPNP,
                                    _task_events_data.ip_to_duration_map[result->get_ip()],
                                    result->get_ip()
                            };
                            _task_events_data.notifier->on_event(bridge_discovered_event);
                        }
                    }
                    _done();
                });
            }
        });
    }
}  // namespace huesdk
