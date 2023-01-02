/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <string>
#include <memory>
#include <vector>

#include "events/IBridgeDiscoveryEventNotifier.h"
#include "events/BridgeDiscoveryTaskEventsData.h"
#include "method/BridgeDiscoveryMethodUtil.h"

#include "support/threading/Job.h"
#include "support/network/http/HttpRequestError.h"
#include "support/network/http/IHttpResponse.h"
#include "support/util/Uuid.h"

using Task = support::JobTask;

namespace huesdk {

    class BridgeDiscoveryNupnpTask : public Task {
    public:
        /**
         Constructor
         @param url Hue portal URL
         */
        explicit BridgeDiscoveryNupnpTask(
                const std::string &url,
                const boost::uuids::uuid& request_id,
                const std::shared_ptr<IBridgeDiscoveryEventNotifier>& notifier);

        /**
         @see Job.h
         */
        void execute(CompletionHandler completion_handler) override;

        /**
         Gets task execution result
         @return const reference to result array
         */
        const std::vector<std::shared_ptr<BridgeDiscoveryResult>> &get_result() const;

    private:
        std::string _url;
        std::vector<std::shared_ptr<BridgeDiscoveryResult>> _results;
        BridgeDiscoveryTaskEventsData _task_events_data;
    };

}  // namespace huesdk


