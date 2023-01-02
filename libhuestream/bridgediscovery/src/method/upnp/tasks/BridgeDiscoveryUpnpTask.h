/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <string>
#include <vector>
#include <memory>

#include "support/threading/Job.h"
#include "support/chrono/Timer.h"
#include "support/util/Uuid.h"

#include "BridgeDiscoveryUpnpListenTask.h"
#include "BridgeDiscoveryUpnpSendTask.h"
#include "events/BridgeDiscoveryEvents.h"
#include "events/BridgeDiscoveryTaskEventsData.h"
#include "method/BridgeDiscoveryMethodUtil.h"

using Task = support::JobTask;

#include "support/network/sockets/_test/SocketUdpDelegator.h"
#define SOCKET_UDP support::SocketUdpDelegator

namespace huesdk {

    class BridgeDiscoveryUpnpTask : public Task {
    public:
        BridgeDiscoveryUpnpTask(
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
        /**
         Runs the job with BridgeDiscoveryUpnpListenTask. Will be continuously invoked until socket is closed.
         After socket is closed, parses the listen results and signals of the task completion
         */
        void start_listen_task();

        std::string _unparsed_listen_result;
        // Timer has to be destroyed before socket to prevent data race between timer event lambda and socket destruction
        std::unique_ptr<SOCKET_UDP> _socket;
        std::unique_ptr<support::Timer> _timeout_timer;
        std::shared_ptr<support::Job<BridgeDiscoveryUpnpSendTask>> _send_job;
        std::vector<std::shared_ptr<BridgeDiscoveryResult>> _unfiltered_results;
        std::vector<std::shared_ptr<BridgeDiscoveryResult>> _filtered_results;
        CompletionHandler _done;
        BridgeDiscoveryTaskEventsData _task_events_data;
    };

}  // namespace huesdk


