/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#ifndef LIBHUESTREAM_MOCKFULLCONFIGRETRIEVER_H
#define LIBHUESTREAM_MOCKFULLCONFIGRETRIEVER_H

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <memory>
#include <functional>
#include <huestream/connect/ConnectionFlow.h>

using namespace testing;
using namespace huestream;



class MockConfigRetriever : public IConfigRetriever {

public:
    BridgePtr Bridge;
    RetrieveCallbackHandler RetrieveCallback;
    FeedbackHandler Feedback;
    void ExecuteRetrieveCallback(OperationResult result, BridgePtr bridge) {
        if (RetrieveCallback == nullptr)
            FAIL();
        RetrieveCallback(result, bridge);
    }
    MOCK_METHOD3(Execute, bool(BridgePtr bridge, RetrieveCallbackHandler cb, FeedbackHandler fh));
    MOCK_METHOD1(OnBridgeMonitorEvent, void(const FeedbackMessage& message));
    MOCK_METHOD0(IsSupportingClipV2, bool());
    MOCK_METHOD0(RefreshBridgeConnection, void());

};

#endif //LIBHUESTREAM_MOCKFULLCONFIGRETRIEVER_H
