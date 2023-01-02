/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#ifndef HUESTREAM_COMMON_TIME_TIMEMANAGER_H_
#define HUESTREAM_COMMON_TIME_TIMEMANAGER_H_

#include "huestream/common/time/ITimeManager.h"

#include <chrono>
#include <atomic>

namespace huestream {

    class TimeManager : public ITimeManager {
    public:
        TimeManager();

        void Sleep(int64_t duration) override;

        int64_t Now() override;

        void UpdateTime() override;

    protected:
        std::atomic<int64_t> _now;
        std::chrono::steady_clock::time_point _start;
    };

}  // namespace huestream

#endif  // HUESTREAM_COMMON_TIME_TIMEMANAGER_H_
