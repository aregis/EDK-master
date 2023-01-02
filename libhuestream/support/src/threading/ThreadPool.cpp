/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <algorithm>
#include <string>

#include "support/threading/ThreadPool.h"
#include "support/threading/Thread.h"
#include "support/util/VectorOperations.h"

namespace support {

    ThreadPool::ThreadPool(size_t workers, bool finish_tasks_on_close, std::string name)
            : _close(false)
            , _should_finish_tasks_on_close(finish_tasks_on_close)
            , _last_tick_time_point{std::chrono::steady_clock::now()}
            , _tick_interval{std::chrono::seconds(1)} {
        for (size_t i = 0; i < workers; i++) {
            const auto thread_name = name.empty() ? name : name + "-" + std::to_string(i + 1);
            _threads.emplace_back(std::make_unique<support::Thread>(thread_name, std::bind(&ThreadPool::event_loop, this)));
        }
    }

    void ThreadPool::event_loop() {
        std::unique_lock<std::mutex> tasks_lock(_tasks_mutex);
        while (!_close || (!_tasks.empty() && _should_finish_tasks_on_close)) {
            _tasks_condition.wait_for(tasks_lock, _tick_interval.load(), [&] () -> bool {
                return (!_tasks.empty() || _close);
            });

            ThreadPoolTask task;
            if (!_tasks.empty()) {
                task = _tasks.front();
                _tasks.pop();
            }

            tasks_lock.unlock();

            if (task) {
                call_and_ignore_exception(task);
                task = {};
            }

            process_tick();
            tasks_lock.lock();
        }
    }
    
    ThreadPool::~ThreadPool() {
        shutdown();
    }

    void ThreadPool::shutdown() {
        {
            std::lock_guard<std::mutex> tasks_lock(_tasks_mutex);
            _close = true;
            _tasks_condition.notify_all();
        }

        for (auto&& worker : _threads) {
            worker->join();
        }
        _threads.clear();
    }

    void ThreadPool::set_tick_interval(std::chrono::milliseconds period) {
        _tick_interval = period;
    }

    Subscription ThreadPool::subscribe_for_tick_events(std::function<void()> handler) {
        return _tick_signal.connect(handler);
    }

    void ThreadPool::process_tick() {
        bool tick_once = false;
        {
            std::lock_guard<std::mutex> lock_guard{_last_tick_time_point_mutex};

            const auto current_time_point = std::chrono::steady_clock::now();
            if (_last_tick_time_point + _tick_interval.load() < current_time_point) {
                _last_tick_time_point = current_time_point;
                tick_once = true;
            }
        }

        if (tick_once) {
            _tick_signal();
        }
    }
}  // namespace support
