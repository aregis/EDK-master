/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#ifndef LIBHUESTREAM_MOCKMESSAGEDISPATCHER_H
#define LIBHUESTREAM_MOCKMESSAGEDISPATCHER_H

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "huestream/connect/MessageDispatcher.h"

namespace huestream {
    class MockMessageDispatcher : public IMessageDispatcher {
    public:
        MOCK_METHOD1(Queue, void(DispatchAction action));
        MOCK_METHOD1(Execute, void(bool useThisTread));
        MOCK_METHOD0(Stop, void());
    };
}

#endif //LIBHUESTREAM_MOCKMESSAGEDISPATCHER_H
