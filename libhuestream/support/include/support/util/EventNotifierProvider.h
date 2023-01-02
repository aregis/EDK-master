/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include "support/util/EventNotifier.h"
#include "support/util/Provider.h"

namespace support {
    using EventNotifierProvider = Provider<OptionalEventNotifier>;
    using ScopedEventNotifierProvider = ScopedProvider<OptionalEventNotifier>;
}  // namespace support

template<>
struct default_object<support::OptionalEventNotifier> {
    static support::OptionalEventNotifier get() {
        return support::OptionalEventNotifier{};
    }
};