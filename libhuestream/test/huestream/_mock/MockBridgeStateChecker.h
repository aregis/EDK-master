//
// Created by nlv15377 on 7/3/2017.
//

#ifndef HUESDK_MOCKBRIDGESTATECHECKER_H
#define HUESDK_MOCKBRIDGESTATECHECKER_H

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "huestream/connect/IBridgeStreamingChecker.h"


namespace huestream {
    class MockBridgeStateChecker : public IBridgeStreamingChecker {
    public:
        MOCK_METHOD1(Check, void(BridgePtr bridge));
        MOCK_METHOD1(SetFeedbackMessageCallback, void(std::function<void(const huestream::FeedbackMessage &)> callback));
    };
}
#endif //HUESDK_MOCKBRIDGESTATECHECKER_H
