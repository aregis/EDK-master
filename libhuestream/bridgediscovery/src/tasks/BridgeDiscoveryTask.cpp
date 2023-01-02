/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <unordered_set>
#include <utility>

#include "support/logging/Log.h"

#include "bridgediscovery/IBridgeDiscoveryCallback.h"

#include "tasks/BridgeDiscoveryTask.h"

using Task = support::JobTask;
using std::vector;
using std::unordered_set;
using std::string;
using std::unique_ptr;
using std::to_string;
using std::none_of;
using std::move;
using std::make_shared;
using std::pair;

namespace huesdk {
    BridgeDiscoveryTask::BridgeDiscoveryTask(vector<unique_ptr<IBridgeDiscoveryMethod>> &&discovery_methods) {
        for (auto &&method : discovery_methods) {
            _current_discovery_methods.push(std::move(method));
        }
    }

    void BridgeDiscoveryTask::execute(CompletionHandler done) {
        _done = move(done);
        start_next_search_method();
    }

    vector<std::shared_ptr<BridgeDiscoveryResult>> BridgeDiscoveryTask::get_results() const {
        vector<std::shared_ptr<BridgeDiscoveryResult>> return_value;
        return_value.reserve(_results.size());

        for (const auto &result_pair : _results) {
            return_value.push_back(result_pair.second);
        }

        return return_value;
    }

    void BridgeDiscoveryTask::start_next_search_method() {
        if (!_current_discovery_methods.empty()) {
            const auto &discovery_method = _current_discovery_methods.front();

            HUE_LOG << HUE_CORE << HUE_DEBUG << "BridgeDiscovery: perform search method "
                    << discovery_method->get_type() << HUE_ENDL;

            discovery_method->search(make_shared<BridgeDiscoveryCallback>([this](const vector<std::shared_ptr<BridgeDiscoveryResult>> &discovery_results, BridgeDiscoveryReturnCode /*return_code*/) {
                _executor.execute([this, discovery_results]() {
                    for (const auto &result_entry : discovery_results) {
                        HUE_LOG << HUE_CORE << HUE_DEBUG << "BridgeDiscovery: result found -> unique id: "
                                << string(result_entry->get_unique_id()) << ", ip: " << string(result_entry->get_ip())
                                << ", model ID: " << string(result_entry->get_model_id()) << HUE_ENDL;
                    }

                    process_results_and_continue(discovery_results);
                });
            }));

        } else {
            HUE_LOG << HUE_CORE << HUE_DEBUG << "BridgeDiscovery: done searching; calling callback";
            _done();
        }
    }

    void BridgeDiscoveryTask::process_results_and_continue(const vector<std::shared_ptr<BridgeDiscoveryResult>>& discovery_results) {
        if (!_current_discovery_methods.empty()) {
            for (const auto &result_entry : discovery_results) {
                _results[result_entry->get_ip()] = result_entry;
            }

            HUE_LOG << HUE_CORE << HUE_DEBUG << "BridgeDiscovery: done performing search method "
                    << _current_discovery_methods.front()->get_type() << "; total results found: "
                    << to_string(discovery_results.size())
                    << HUE_ENDL;

            _current_discovery_methods.pop();
            start_next_search_method();
        }
    }

    void BridgeDiscoveryTask::stop() {
        if (!_current_discovery_methods.empty()) {
            _current_discovery_methods.front()->stop();

            while (!_current_discovery_methods.empty()) {
                _current_discovery_methods.pop();
            }
        }
    }
}  // namespace huesdk
