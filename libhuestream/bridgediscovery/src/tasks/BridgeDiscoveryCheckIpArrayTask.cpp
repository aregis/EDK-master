/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include "tasks/BridgeDiscoveryCheckIpArrayTask.h"
#include "bridgediscovery/BridgeDiscoveryCheckIpTask.h"
#include "support/logging/Log.h"

using Task = support::JobTask;
using std::vector;
using std::string;

namespace huesdk {
    BridgeDiscoveryCheckIpArrayTask::BridgeDiscoveryCheckIpArrayTask(const vector<std::shared_ptr<BridgeDiscoveryResult>> &input_results, const ResultCallback &result_callback)
            : _input_results(input_results), _jobs_count(0), _callback(result_callback) {
    }

    BridgeDiscoveryCheckIpArrayTask::BridgeDiscoveryCheckIpArrayTask(const vector<std::string> &ips, const ResultCallback &result_callback)
            : BridgeDiscoveryCheckIpArrayTask(vector<std::shared_ptr<BridgeDiscoveryResult>>{}, result_callback) {
        for (auto ip : ips) {
            _input_results.push_back(std::make_shared<BridgeDiscoveryResult>(ip));
        }
    }

    void BridgeDiscoveryCheckIpArrayTask::execute(Task::CompletionHandler done) {
        if (_input_results.empty()) {
            done();
        } else {
            _jobs_count = _input_results.size();
            for (const auto &ip : _input_results) {
                auto check_ip_job = create_job<BridgeDiscoveryCheckIpTask>(ip);
                check_ip_job->run([this, done](BridgeDiscoveryCheckIpTask *task) {
                    auto task_result = task->get_result();
                    if (task_result.reachable && !task_result.unique_id.empty() && task_result.is_bridge) {
                        _result.push_back(task_result);
                        if (_callback) {
                            _callback(task_result);
                        }
                    }

                    --_jobs_count;
                    if (_jobs_count == 0) {
                        HUE_LOG << HUE_CORE << HUE_DEBUG
                                << "BridgeDiscoveryCheckIpArrayTask: done processing; calling callback -> "
                                << static_cast<int64_t>(_result.size()) << " results found" << HUE_ENDL;
                        done();
                    }
                });
            }
        }
    }

    vector<BridgeDiscoveryIpCheckResult> &BridgeDiscoveryCheckIpArrayTask::get_result() {
        return _result;
    }

}  // namespace huesdk
