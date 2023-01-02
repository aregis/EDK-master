/*******************************************************************************
Copyright (C) 2019 Signify Holding
All Rights Reserved.
********************************************************************************/
#pragma once

#include <functional>
#include <memory>
#include <vector>

#include "support/util/Operation.h"
#include "support/util/Provider.h"
#include "support/threading/Executor.h"
#include "support/threading/ThreadPool.h"
#include "support/threading/ConditionVariable.h"
#include "support/threading/detail/TaskSchedule.h"

namespace support {

    /**
     * Provides functionality for executing functions withing an thread pool.
     */
    class ThreadPoolExecutor : public Executor {
    public:
        enum class ShutdownPolicy {
            CANCEL_ALL,
            WAIT_ALL,
            KEEP_RUNNING
        };

        /**
         * Construct an executor that operates on global thread pool
         */
        explicit ThreadPoolExecutor(ShutdownPolicy shutdown_policy = ShutdownPolicy::CANCEL_ALL);

        /**
         * Constructor.
         * @param thread_pool Thread poool used to execute given requests
         */
        explicit ThreadPoolExecutor(std::shared_ptr<ThreadPool> thread_pool, ShutdownPolicy shutdown_policy = ShutdownPolicy::CANCEL_ALL);

        /**
         * Destructor.
         * Cancels all posted calls and waits for non cancelable executions to complete.
         */
        ~ThreadPoolExecutor() override;

        ThreadPoolExecutor(const ThreadPoolExecutor&) = delete;
        ThreadPoolExecutor& operator=(const ThreadPoolExecutor&) = delete;

        /**
         * Requests item execution on the thread pool. Item is put the the end of the queue.
         * @param invocable Function to be executed
         * @param operation_type Specifies if executor allowed to disacard this request on cancellation and destruction.
         */
        support::Operation execute(std::function<void()> invocable, OperationType operation_type = OperationType::CANCELABLE) override;

        /**
         * Schedule a task after certain point of time. After each task execution, executor filters all the tasks that need
         * to be executed after that point of time and executes them using Executor::schedule method.
         * @param time_point time point after which the task could be executed
         * @param invocable Function to be executed.
         */
        void schedule(std::chrono::steady_clock::time_point time_point, std::function<void()> invocable, OperationType operation_type = OperationType::CANCELABLE) override;

        /**
         * Wait for all requests to be executed.
         * Blocks until there is no requests to process.
         * All incoming requests during wait_all() will be also processed.
         */
        void wait_all() override;

        /**
         * Discards cancelable requests and waits other requests to be executed.
         * Blocks until there is no requests to process.
         * All incoming requests during cancel_all() will be also processed.
         */
        void cancel_all() override;

        /**
         * Stops all incoming requests processing, discards cancelable requests and waits other requests to be executed.
         * Blocks until there is no requests to process.
         */
        void shutdown() override;

        /**
         * Gets thread pool used for requests execution.
         * @return working operational queue pointer
         */
        std::shared_ptr<ThreadPool> get_thread_pool() const;

    private:
        void shutdown(ShutdownPolicy shutdown_policy);

        class Operation;
        std::vector<std::shared_ptr<Operation>> take_operations();

        struct SharedData {
            std::shared_ptr<std::mutex> _mutex = std::make_shared<std::mutex>();
            std::vector<std::shared_ptr<Operation>> _operations;
            detail::TaskSchedule _task_schedule;
        };
        std::shared_ptr<SharedData> _shared_data;
        std::shared_ptr<ThreadPool> _thread_pool;
        support::ConditionVariable<int> _waiting_condition{0};
        bool _is_shutdown = false;
        support::Subscription _task_executed_subscription;
        ThreadPoolExecutor::ShutdownPolicy _shutdown_policy = ThreadPoolExecutor::ShutdownPolicy::CANCEL_ALL;
    };
}  // namespace support

