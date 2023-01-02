/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <memory>
#include <vector>

#include "events/IBridgeDiscoveryEventNotifier.h"
#include "events/BridgeDiscoveryTaskEventsData.h"
#include "method/BridgeDiscoveryMethodUtil.h"

#include "support/threading/Job.h"
#include "support/threading/QueueExecutor.h"
#include "support/util/Uuid.h"

using Task = support::JobTask;

namespace huesdk {

    class BridgeDiscoveryIpscanTask : public Task {
    public:
        BridgeDiscoveryIpscanTask(
                const boost::uuids::uuid& request_id,
                const std::shared_ptr<IBridgeDiscoveryEventNotifier>& notifier);

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
        const std::vector<std::shared_ptr<BridgeDiscoveryResult>> &get_result() const;

    private:
        std::vector<std::shared_ptr<BridgeDiscoveryResult>> _results;
        support::QueueExecutor _executor;
        std::atomic<bool> _stopped_by_user;
        BridgeDiscoveryTaskEventsData _task_events_data;
    };

}  // namespace huesdk


