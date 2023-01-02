/*******************************************************************************
Copyright (C) 2019 Signify Holding
All Rights Reserved.
********************************************************************************/

#pragma once

#include <functional>
#include <memory>
#include <utility>
#include <vector>

#include "support/threading/Executor.h"
#include "support/threading/detail/CancellationManager.h"

namespace support {
    template <typename T>
    class Future;
    template <typename T>
    class CompositeFuture;
    template <typename T>
    class Promise;

    namespace detail {
        class FutureScheduler {
        public:
            virtual ~FutureScheduler() = default;
            virtual void schedule(std::function<void()> invocable) = 0;
        };

        class LocalScheduler : public FutureScheduler {
        public:
            void schedule(std::function<void()> invocable) override {
                _invocable = std::move(invocable);
            }

            std::function<void()> move_invocable() {
                return std::move(_invocable);
            }
        private:
            std::function<void()> _invocable;
        };

        class ExecutorScheduler : public FutureScheduler {
        public:
            explicit ExecutorScheduler(std::shared_ptr<Executor> executor) : _executor{executor} {}
            void schedule(std::function<void()> invocable) override {
                _executor->execute(std::move(invocable));
            }
        private:
            std::shared_ptr<Executor> _executor;
        };

        template <typename T>
        Promise<T> schedule(FutureScheduler& scheduler,
                            std::shared_ptr<Executor> executor,
                            std::shared_ptr<CancellationManager> cancellation_manager,
                            std::function<T(void)> function);
        Promise<void> schedule(FutureScheduler& scheduler,
                               std::shared_ptr<Executor> executor,
                               std::shared_ptr<CancellationManager> cancellation_manager,
                               std::function<support::Future<void>(void)> function);
        Promise<void> schedule(FutureScheduler& scheduler,
                               std::shared_ptr<Executor> executor,
                               std::shared_ptr<CancellationManager> cancellation_manager,
                               std::function<support::CompositeFuture<void>(void)> function);
        template <typename T>
        Promise<T> schedule(FutureScheduler& scheduler, std::shared_ptr<Executor> executor,
                            std::shared_ptr<CancellationManager> cancellation_manager,
                            std::function<support::Future<T>(void)> function);
        template <typename T>
        Promise<std::vector<T>> schedule(FutureScheduler& scheduler, std::shared_ptr<Executor> executor,
                                         std::shared_ptr<CancellationManager> cancellation_manager,
                                         std::function<support::CompositeFuture<T>(void)> function);
    }  // namespace detail
}  // namespace support