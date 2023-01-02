/*******************************************************************************
Copyright (C) 2019 Signify Holding
All Rights Reserved.
********************************************************************************/

#ifndef HUESTREAM_CONNECT_ICONNECTIONMONITOR_H_
#define HUESTREAM_CONNECT_ICONNECTIONMONITOR_H_

#include "huestream/connect/FeedbackMessage.h"
#include "huestream/common/data/Bridge.h"
#include <memory>

namespace huestream {

    class IConnectionMonitor {
    public:
        virtual ~IConnectionMonitor() = default;

        virtual void Start(BridgePtr bridge, int interval_msec) = 0;
        virtual void Stop() = 0;
        virtual void SetFeedbackMessageCallback(std::function<void(const huestream::FeedbackMessage &)> callback) = 0;
    };

typedef std::shared_ptr<IConnectionMonitor> ConnectionMonitorPtr;

}  // namespace huestream

#endif  // HUESTREAM_CONNECT_ICONNECTIONMONITOR_H_
