/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <vector>
#include <queue>
#include <memory>
#include <unordered_map>
#include <string>

#include "support/threading/Job.h"

#include "method/BridgeDiscoveryMethodUtil.h"

using Task = support::JobTask;

namespace huesdk {

    class BridgeDiscoveryTask : public Task {
    public:
        /**
         Constructor
         @param discovery_methods discovery methods to use for bridge searching
         */
        explicit BridgeDiscoveryTask(std::vector<std::unique_ptr<IBridgeDiscoveryMethod>> &&discovery_methods);

        /**
         @see Job.h
         */
        void execute(CompletionHandler completion_handler) override;

        /**
         @see Job.h
         */
        void stop() override;

        /**
         Gets task execution result
         @return const reference to result array
         */
        std::vector<std::shared_ptr<BridgeDiscoveryResult>> get_results() const;

    private:
        /**
         Starts executing next search method in the queue. If the queue is empty, task finishes.
         */
        void start_next_search_method();

        /**
         Saves discovery results to result map. Pops current discovery result from the queue.
         @param discovery_results discovery method results to be processed
         */
        void process_results_and_continue(const std::vector<std::shared_ptr<BridgeDiscoveryResult>> &discovery_results);

        support::QueueExecutor _executor;
        std::queue<std::unique_ptr<IBridgeDiscoveryMethod>> _current_discovery_methods;

        std::unordered_map<std::string, std::shared_ptr<BridgeDiscoveryResult>> _results;

        CompletionHandler _done;
    };

}  // namespace huesdk


