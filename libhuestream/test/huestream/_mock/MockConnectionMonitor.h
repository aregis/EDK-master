/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/
#ifndef LIBHUESTREAM_TEST_MOCKCONNECTIONMONITOR_H_
#define LIBHUESTREAM_TEST_MOCKCONNECTIONMONITOR_H_

#include "gmock/gmock.h"

#include <memory>

#include "huestream/connect/IConnectionMonitor.h"

namespace huestream {

    class MockConnectionMonitor : public IConnectionMonitor {
    public:
        FeedbackMessageCallback callback;

        MockConnectionMonitor() {}
        MOCK_METHOD2(Start, void(BridgePtr bridge, int interval_msec));
        MOCK_METHOD0(Stop, void());
        MOCK_METHOD1(SetFeedbackMessageCallback, void(std::function<void(const huestream::FeedbackMessage &)> callback));
    };

    class MockWrapperConnectionMonitor : public IConnectionMonitor
    {
    public:
        explicit MockWrapperConnectionMonitor(
                const std::shared_ptr<MockConnectionMonitor>& mock)
          : _mock(mock) {}

        void Start(BridgePtr bridge, int interval_msec) {
            _mock->Start(bridge, interval_msec);
        }

        void Stop() {
            _mock->Stop();
        }

        void SetFeedbackMessageCallback(std::function<void(const huestream::FeedbackMessage &)> callback) {
            _mock->SetFeedbackMessageCallback(callback);
        }

    private:
        std::shared_ptr<MockConnectionMonitor> _mock;
    };

}// namespace huestream
#endif     // LIBHUESTREAM_TEST_MOCKCONNECTIONMONITOR_H_
