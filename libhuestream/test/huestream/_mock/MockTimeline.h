/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/
#ifndef LIBHUESTREAM_TEST_MOCKTIMELINE_H_
#define LIBHUESTREAM_TEST_MOCKTIMELINE_H_

#include "gmock/gmock.h"
#include "huestream/effect/lightscript/ITimeline.h"

namespace huestream {

    class MockTimeline : public ITimeline {
    public:
        MOCK_METHOD0(Start, void());

        MOCK_METHOD0(Pause, void());

        MOCK_METHOD0(Resume, void());

        MOCK_METHOD0(Stop, void());

        MOCK_METHOD1(SetPosition, void(int64_t position));

        MOCK_METHOD0(IsRunning, bool());

        MOCK_METHOD0(Now, int64_t());

        MOCK_METHOD1(SetLength, void(int64_t length));

        MOCK_METHOD0(GetLength, int64_t());

        MOCK_METHOD1(RegisterTimelineStateChangedHandler, void(TimelineStateChangedHandlerPtr handler));
        MOCK_METHOD1(RegisterTimelineStateChangedCallback, void(TimelineStateChangedCallback callback));
    };
}


#endif   // LIBHUESTREAM_TEST_MOCKTIMELINE_H_
