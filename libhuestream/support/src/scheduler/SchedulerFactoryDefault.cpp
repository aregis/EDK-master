/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <memory>

#include "support/scheduler/Scheduler.h"
#include "support/scheduler/SchedulerFactory.h"
#include "support/util/MakeUnique.h"

template <>
std::unique_ptr<support::Scheduler> huesdk_lib_default_factory<support::Scheduler>(
        unsigned timer_interval_ms) {
    return support::make_unique<support::Scheduler>(timer_interval_ms);
}