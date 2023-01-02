/*******************************************************************************
Copyright (C) 2019 Signify Holding
All Rights Reserved.
********************************************************************************/

#ifndef HUESTREAM_CONNECT_CONNECTIONMONITOR_H_
#define HUESTREAM_CONNECT_CONNECTIONMONITOR_H_

#include "huestream/connect/IConnectionMonitor.h"
#include "huestream/connect/IBridgeStreamingChecker.h"

#include <thread>
#include <memory>
#include <mutex>
#include <condition_variable>


namespace huestream {

class ConnectionMonitor: public IConnectionMonitor {
public:
   explicit ConnectionMonitor(BridgeStateCheckerPtr bridgeStateChecker);

    void Start(BridgePtr bridge, int interval_msec) override;
    void Stop() override;
    void SetFeedbackMessageCallback(std::function<void(const huestream::FeedbackMessage &)> callback) override;

 private:
    void MonitorThread();
    BridgeStateCheckerPtr _bridgeStateChecker;
    BridgePtr _bridge;
    std::shared_ptr<std::thread> _monitorThread;
    bool _running;
    std::mutex _mutex;
    std::condition_variable _cv;

    int _interval_msec;
};

}  // namespace huestream

#endif  // HUESTREAM_CONNECT_CONNECTIONMONITOR_H_
