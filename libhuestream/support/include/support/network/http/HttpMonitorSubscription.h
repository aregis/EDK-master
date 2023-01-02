/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include "support/network/http/HttpMonitorObserver.h"

namespace support {

    class HttpMonitorSubscription {
    public:
        /**
         Class constructor. Registers observer as HttpMonitor listener
         @param observer observer that has to be registered.
         */
        explicit HttpMonitorSubscription(const HttpMonitorObserver& observer);

        /**
         Class destructor. Un-registers observer
         */
        ~HttpMonitorSubscription();

    private:
        /** Reference to the registered observer */
        const HttpMonitorObserver& _observer;
    };

}  // namespace support

