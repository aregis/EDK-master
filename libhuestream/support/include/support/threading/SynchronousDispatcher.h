/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <functional>
#include <set>
#include <thread>
#include <atomic>

#include "support/threading/ConditionVariable.h"
#include "support/util/Operation.h"

namespace support {
    class SynchronousDispatcher {
    public:
        void shutdown() {
            _is_active = false;
            bool wait_for_idle = true;
            {
                std::lock_guard<std::mutex> lock{_dispacher_thread_ids_mutex};
                wait_for_idle = _dispather_thread_ids.find(std::this_thread::get_id()) == _dispather_thread_ids.end();
            }
            if (wait_for_idle) {
                _idle_condition.wait(0);
            }
        }

        support::Operation post(std::function<void()> invocation) {
            if (_is_active) {
                _idle_condition.perform(support::operations::increment<size_t>);
                {
                    std::lock_guard<std::mutex> lock{_dispacher_thread_ids_mutex};
                    _dispather_thread_ids.insert(std::this_thread::get_id());
                }
                invocation();
                {
                    std::lock_guard<std::mutex> lock{_dispacher_thread_ids_mutex};
                    _dispather_thread_ids.erase(std::this_thread::get_id());
                }
                _idle_condition.perform(support::operations::decrement<size_t>);
            }
            return {};
        }

    private:
        std::atomic<bool> _is_active{true};
        std::mutex _dispacher_thread_ids_mutex;
        std::set<std::thread::id>  _dispather_thread_ids;
        support::ConditionVariable<size_t > _idle_condition{0};
    };
}  // namespace support