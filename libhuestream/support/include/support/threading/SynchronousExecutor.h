/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <condition_variable>
#include <memory>

#include "support/threading/Executor.h"
#include "support/threading/ConditionVariable.h"
#include "support/threading/ThreadPoolExecutor.h"

namespace support {
    class ThreadPool;

    class SynchronousExecutor : public Executor {
    public:
        SynchronousExecutor() = default;
        explicit SynchronousExecutor(std::shared_ptr<support::ThreadPool> thread_pool);

        support::Operation execute(std::function<void()> invocable, OperationType operation_type = OperationType::CANCELABLE) override;

        void schedule(std::chrono::steady_clock::time_point time_point,
                std::function<void()> invocable, OperationType operation_type = OperationType::CANCELABLE) override;

        void wait_all() override;
        void cancel_all() override;
        void shutdown() override;

    private:
        std::mutex _mutex;
        bool _is_active{true};
        size_t _invoke_counter{0};
        std::condition_variable _invoke_counter_condition;
        ThreadPoolExecutor _executor;
    };
}  // namespace support