/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <chrono>
#include <memory>
#include <string>
#include <unordered_map>

#include <boost/optional.hpp>
#include <boost/uuid/uuid.hpp>

#include "events/IBridgeDiscoveryEventNotifier.h"

namespace huesdk {

    struct BridgeDiscoveryTaskEventsData {
        boost::optional<std::chrono::time_point<std::chrono::system_clock>> start_of_task;
        boost::uuids::uuid request_id;
        std::shared_ptr<IBridgeDiscoveryEventNotifier> notifier;
        std::unordered_map<std::string, std::chrono::duration<double>> ip_to_duration_map;
    };

}  // namespace huesdk