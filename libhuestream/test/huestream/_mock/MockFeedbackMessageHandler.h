/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#ifndef LIBHUESTREAM_MOCKFEEDBACKMESSAGEHANDLER_H
#define LIBHUESTREAM_MOCKFEEDBACKMESSAGEHANDLER_H

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "huestream/connect/IFeedbackMessageHandler.h"

namespace huestream {
    class MockFeedbackMessageHandler : public IFeedbackMessageHandler {
    public:
        MOCK_CONST_METHOD1(NewFeedbackMessage, void(const FeedbackMessage &message));
    };
}


#endif //LIBHUESTREAM_MOCKFEEDBACKMESSAGEHANDLER_H
