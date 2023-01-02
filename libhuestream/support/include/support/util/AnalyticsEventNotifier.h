/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include "support/util/EventNotifier.h"
#include "support/util/EventNotifierProvider.h"

namespace support {

    class AnalyticsEventNotifier {
    public:
        void register_listener(EventNotifier notifier) {
            EventNotifierProvider::set(notifier);
        }
    };

}  // namespace support