/*******************************************************************************
Copyright (C) 2019 Signify Holding
All Rights Reserved.
********************************************************************************/

#include "support/threading/SynchronousExecutor.h"
#include "support/threading/ThreadPool.h"

using support::SynchronousExecutor;

SynchronousExecutor::SynchronousExecutor(std::shared_ptr<support::ThreadPool> thread_pool)
    : _executor{thread_pool} {
}

support::Operation SynchronousExecutor::execute(std::function<void()> invocable, OperationType) {
    bool perform_operation = false;
    {
        std::lock_guard<std::mutex> lock{_mutex};
        perform_operation = _is_active;
        if (perform_operation) {
            ++_invoke_counter;
        }
    }

    if (perform_operation) {
        invocable();
        std::lock_guard<std::mutex> lock{_mutex};
        --_invoke_counter;
        _invoke_counter_condition.notify_all();
    }

    return {};
}

void SynchronousExecutor::schedule(std::chrono::steady_clock::time_point time_point,
              std::function<void()> invocable, OperationType operation_type) {
    _executor.schedule(time_point, invocable, operation_type);
}

void SynchronousExecutor::wait_all() {
    std::unique_lock<std::mutex> lock{_mutex};
    _invoke_counter_condition.wait(lock, [&]{
        return _invoke_counter == 0;
    });
}

void SynchronousExecutor::cancel_all() {
    wait_all();
}

void SynchronousExecutor::shutdown() {
    {
        std::lock_guard<std::mutex> lock{_mutex};
        _is_active = false;
    }
    wait_all();
}