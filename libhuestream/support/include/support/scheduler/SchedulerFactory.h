/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <memory>

#include "Scheduler.h"
#include "support/util/Factory.h"

namespace support {

    using SchedulerFactory = support::Factory<std::unique_ptr<Scheduler>(unsigned)>;

}  // namespace support

