/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <functional>
#include <string>
#include <unordered_map>

#include "boost/optional.hpp"

namespace support {
    using EventNotifier = std::function<void(const std::string&, const std::unordered_map<std::string, std::string>&)>;
    using OptionalEventNotifier = boost::optional<EventNotifier>;
}  // namespace support
