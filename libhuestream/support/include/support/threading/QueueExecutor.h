/*******************************************************************************
Copyright (C) 2019 Signify Holding
All Rights Reserved.
********************************************************************************/
#pragma once

#include <functional>
#include <memory>

#include "support/util/Operation.h"
#include "support/util/Provider.h"
#include "support/threading/Executor.h"
#include "support/threading/OperationalQueue.h"

namespace support {

    /**
     * Provides functionality for executing functions withing an operational queue.
     */
    class QueueExecutor final : public Executor {
    public:
        /**
         * Construct an executor that operates on global operational queue
         */
        QueueExecutor();

        /**
         * Constructor.
         * @param queue Operational queue used to execute given requests
         */
        explicit QueueExecutor(std::shared_ptr<OperationalQueue> queue);
        /**
         * Destructor.
         * Cancels all posted calls and waits for non cancelable executions to complete.
         */
        ~QueueExecutor() override;

        QueueExecutor(const QueueExecutor&) = delete;
        QueueExecutor& operator=(const QueueExecutor&) = delete;

        /**
         * Requests item execution on the operational queue. Item is put the the end of the queue.
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
         * Gets operational queue used for requests execution.
         * @return working operational queue pointer
         */
        std::shared_ptr<OperationalQueue> get_operational_queue() const;

    private:
        void clear(OperationType policy);

        struct Impl;
        std::shared_ptr<Impl> _impl;
    };

    using GlobalQueueExecutor = Provider<std::shared_ptr<QueueExecutor>>;
}  // namespace support

template<>
struct default_object<std::shared_ptr<support::QueueExecutor>> {
    static std::shared_ptr<support::QueueExecutor> get() {
        return std::make_shared<support::QueueExecutor>(
                std::make_shared<support::OperationalQueue>());
    }
};
