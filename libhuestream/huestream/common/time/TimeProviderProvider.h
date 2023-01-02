/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#ifndef HUESTREAM_COMMON_TIME_TIMEPROVIDERPROVIDER_H_
#define HUESTREAM_COMMON_TIME_TIMEPROVIDERPROVIDER_H_

#include "support/util/Provider.h"
#include "huestream/common/time/ITimeProvider.h"
#include "huestream/common/time/TimeManager.h"

template <>
struct default_object<huestream::TimeProviderPtr>
{
    static huestream::TimeProviderPtr get() {
        return std::make_shared<huestream::TimeManager>();
    }
};

namespace huestream {
    using TimeProviderProvider = support::Provider<TimeProviderPtr>;
    using ScopedTimeProviderProvider = support::ScopedProvider<TimeProviderPtr>;
}

#endif  // HUESTREAM_COMMON_TIME_TIMEPROVIDERPROVIDER_H_
