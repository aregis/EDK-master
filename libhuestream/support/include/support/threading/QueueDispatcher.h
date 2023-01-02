/*******************************************************************************
Copyright (C) 2019 Signify Holding
All Rights Reserved.
********************************************************************************/

#pragma once

#include <memory>
#include <functional>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <chrono>
#include <future>
#include <queue>

#include "support/util/Operation.h"
#include "support/util/Provider.h"
#include "support/threading/OperationalQueue.h"
#include "support/threading/ThreadPool.h"

namespace support {
    class QueueExecutor;

    /**
     * Provides functionality for dispatching information to external listeners through operational queue.
     */
    class QueueDispatcher final {
    public:
        /**
         * Constructor using global operational queue
         * @param queue Dispatch queue used to execute given requests
         * @param wait_all specifies if the dispatcher should wait posted invocations on shutdown.
         */
        explicit QueueDispatcher(bool wait_all = true);

        /**
         * Constructor.
         * @param queue Dispatch queue used to execute given requests
         * @param wait_all specifies if the dispatcher should wait posted invocations on shutdown.
         */
        explicit QueueDispatcher(std::shared_ptr<OperationalQueue> queue, bool wait_all = true);

        /**
         * Waits for posted invocations to be discarded or finished.
         */
        ~QueueDispatcher();

        QueueDispatcher(const QueueDispatcher&) = delete;
        QueueDispatcher& operator=(const QueueDispatcher&) = delete;

        /**
         * Requests invocation to be performed on the dispatcher thread.
         * @param invocation Function to be executed
         */
        support::Operation post(std::function<void()> invocation);

        /**
         * Stops all incoming requests processing, waits for posted invocations to be discarded or finished.
         * Blocks until there is no requests to process.
         */
        void shutdown();

        /**
         * Retrieve operation queue that is used for executing dispatching tasks
         */
        std::shared_ptr<OperationalQueue> get_operational_queue() const;

    private:
        std::unique_ptr<QueueExecutor> _executor;
        bool _wait_all = true;
    };

    using GlobalQueueDispatcher = Provider<std::shared_ptr<QueueDispatcher>>;
}  // namespace support

template<>
struct default_object<std::shared_ptr<support::QueueDispatcher>> {
    static std::shared_ptr<support::QueueDispatcher> get() {
        return std::make_shared<support::QueueDispatcher>(
                std::make_shared<support::OperationalQueue>(
                        std::make_shared<support::ThreadPool>(1, true, "dispatcher")));
    }
};

