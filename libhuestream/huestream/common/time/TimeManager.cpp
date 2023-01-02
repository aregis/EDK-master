/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <huestream/common/time/TimeManager.h>

#include <thread>

namespace huestream {

    TimeManager::TimeManager() :
            _now(0), _start(std::chrono::steady_clock::now()) {
    }

    void TimeManager::Sleep(int64_t duration) {
        std::this_thread::sleep_for(std::chrono::milliseconds(duration));
    }

    int64_t TimeManager::Now() {
        return _now;
    }

    void TimeManager::UpdateTime() {
        _now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - _start).count();
    }
}  // namespace huestream
