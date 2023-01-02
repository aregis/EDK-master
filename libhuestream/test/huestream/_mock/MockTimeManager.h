/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/
#ifndef LIBHUESTREAM_TEST_MOCKTIMEMANAGER_H_
#define LIBHUESTREAM_TEST_MOCKTIMEMANAGER_H_

#include <huestream/common/time/ITimeManager.h>
#include "gmock/gmock.h"

namespace huestream {

    class MockTimeManager : public ITimeManager {
    public:
        MOCK_METHOD1(Sleep, void(int64_t duration));
        MOCK_METHOD0(Now, int64_t());
        MOCK_METHOD0(UpdateTime, void());
    };

    class MockWrapperTimeManager : public ITimeManager {
    public:
        explicit MockWrapperTimeManager(const std::shared_ptr<MockTimeManager>& mock)
          : _mock(mock) {}

        void Sleep(int64_t duration) {
            _mock->Sleep(duration);
        }

        int64_t Now() {
            return _mock->Now();
        }

        void UpdateTime() {
            return _mock->UpdateTime();
        }
    private:
        std::shared_ptr<MockTimeManager> _mock;
    };
}

#endif   // LIBHUESTREAM_TEST_MOCKTIMEMANAGER_H_
