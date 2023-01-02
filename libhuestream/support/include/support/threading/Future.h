/*******************************************************************************
Copyright (C) 2019 Signify Holding
All Rights Reserved.
********************************************************************************/

#pragma once

#include <functional>
#include <memory>
#include <utility>
#include <vector>

#include "boost/fusion/adapted/struct.hpp"
#include "boost/fusion/include/algorithm.hpp"
#include "boost/fusion/include/adapt_struct.hpp"

#include "support/threading/ThreadPoolExecutor.h"
#include "support/threading/FutureExceptions.h"
#include "support/threading/detail/FutureScheduler.h"
#include "support/threading/detail/FutureData.h"
#include "support/threading/detail/AsyncAPITraits.h"

namespace support {
    static auto FutureCanceledException = std::make_exception_ptr(FutureCanceled{});
    
    namespace detail {
        template <typename T>
        class FutureBase {
        public:
            FutureBase() = default;
            explicit FutureBase(std::shared_ptr<detail::FutureData<T>> data) : _data(std::move(data)) {}

            bool is_valid() const noexcept {
                return _data != nullptr;
            }

            void get() {
                throw_if_invalid();
                wait();
                throw_if_contains_exception();
            }

            void wait() const {
                throw_if_invalid();
                std::unique_lock<std::mutex> lock{_data->_mutex};
                _data->_condition.wait(lock, [this]{
                    return is_done();
                });
                throw_if_promise_broken();
            }

            template<class Rep, class Period>
            void wait_for(const std::chrono::duration<Rep, Period>& timeout_duration) const {
                wait_until(std::chrono::system_clock::now() + timeout_duration);
            }

            template<class Clock, class Duration>
            void wait_until(const std::chrono::time_point<Clock, Duration>& timeout_time) const {
                throw_if_invalid();
                std::unique_lock<std::mutex> lock{_data->_mutex};
                const auto result = _data->_condition.wait_until(lock, timeout_time, [this]{
                    return is_done();
                });
                throw_if_promise_broken();
                if (!result) {
                    throw WaitTimeoutException{};
                }
            }

            bool is_ready() const noexcept {
                return _data->_is_set;
            }

            template <typename Function>
            auto then(Function&& function) {
                throw_if_invalid();
                std::lock_guard<std::mutex> lock{_data->_mutex};
                if (_data->_is_set) {
                    if (_data->_exception) {
                        LocalScheduler scheduler;
                        auto promise = detail::schedule(scheduler, _data->_executor, _data->_cancellation_manager, std::forward<Function>(function));
                        promise.set_exception(_data->_exception);
                        return promise.get_future();
                    } else {
                        ExecutorScheduler scheduler{_data->_executor};
                        return detail::schedule(scheduler, _data->_executor, _data->_cancellation_manager, std::forward<Function>(function)).get_future();
                    }
                } else {
                    LocalScheduler scheduler;
                    auto promise = detail::schedule(scheduler, _data->_executor, _data->_cancellation_manager, std::forward<Function>(function));
                    _data->_continuations.emplace_back([promise, invocable = scheduler.move_invocable()]() mutable {
                        promise.get_executor()->execute(std::move(invocable));
                    });
                    return promise.get_future();
                }
            }

            void cancel() {
                throw_if_invalid();
                _data->_cancellation_manager->cancel(FutureCanceledException);
                _data->_executor->cancel_all();
                set_future_data_exception(*_data, FutureCanceledException);
            }

            std::shared_ptr<detail::FutureData<T>> get_data() const noexcept {
                return _data;
            }

            std::shared_ptr<Executor> get_executor() const noexcept {
                return get_data()->_executor;
            }

            /**
             * Subscribe to cancellation event, which is emitted if exception is caught by promise.
             * CancellationDelegate is normally invoked from background thread.
             * If Future has already been cancelled, then CancellationDelegate will be invoked immediately
             * in a callee Thread.
             * @Note: Cancellation event is not emitted if Promise wasn't created.
             * @param cancellation_delegate
             * @return Subscription object
             */
            support::Subscription subscribe_for_cancel(CancellationDelegate cancellation_delegate) {
                return get_data()->_cancellation_manager->subscribe(std::move(cancellation_delegate));
            }

        protected:
            bool is_done() const noexcept {
                return _data->_is_set || _data->_is_promise_destroyed;
            }

            void throw_if_invalid() const {
                if (!is_valid()) {
                    throw InvalidFutureException{};
                }
            }

            void throw_if_promise_broken() const {
                if (_data->_is_promise_destroyed && !_data->_is_set) {
                    throw BrokenPromiseException{};
                }
            }

            void throw_if_contains_exception() const {
                if (_data->_exception) {
                    std::rethrow_exception(_data->_exception);
                }
            }

        private:
            std::shared_ptr<detail::FutureData<T>> _data;
        };

        template <typename T>
        class CompositeFutureBase {
        public:
            explicit CompositeFutureBase(std::vector<support::Future<T>> futures) : _futures(std::move(futures)) {}

            void wait() {
                for (auto&& future : _futures) {
                    future.wait();
                }
            }

            template <class Duration>
            support::Future<T> wait_until(Duration duration) {
                for (auto&& future : _futures) {
                    future.wait_until(duration);
                }
            }

            template <class Period>
            support::Future<T> wait_for(Period timeout_duration) {
                return wait_until(std::chrono::system_clock::now() + timeout_duration);
            }

            bool is_ready() const noexcept {
                bool return_value = true;
                for (auto&& future : _futures) {
                    return_value &= future.is_ready();
                }
                return return_value;
            }

            void cancel() {
                for (auto&& future : _futures) {
                    future.cancel();
                }
            }

            std::vector<support::Future<T>> get_futures() noexcept {
                return _futures;
            }

            std::shared_ptr<Executor> get_executor() const noexcept {
                return _futures.size() ? _futures.begin()->get_executor() : std::shared_ptr<Executor>{};
            }

        protected:
            template <typename Function, typename FunctionReturnType>
            CompositeFuture<FunctionReturnType> then(Function function) {
                std::vector<support::Future<FunctionReturnType>> futures;
                for (auto&& future : _futures) {
                    futures.emplace_back(future.then(function));
                }
                return CompositeFuture<FunctionReturnType>{futures};
            }

        private:
            std::vector<support::Future<T>> _futures;
        };

        template <typename T>
        class PromiseBase {
        public:
            PromiseBase() : PromiseBase(std::make_shared<ThreadPoolExecutor>(ThreadPoolExecutor::ShutdownPolicy::KEEP_RUNNING), std::make_shared<CancellationManager>()) {}
            PromiseBase(std::shared_ptr<Executor> executor, std::shared_ptr<CancellationManager> cancellation_manager)
                : _data{create_promise_data(executor, cancellation_manager)} {}

            support::Future<T> get_future() const noexcept {
                return support::Future<T>{get_future_data()};
            }

            std::shared_ptr<Executor> get_executor() const noexcept {
                return get_future_data()->_executor;
            }

            std::shared_ptr<CancellationManager> get_cancellation_manager() const noexcept {
                return get_future_data()->_cancellation_manager;
            }

            void set_exception(std::exception_ptr ex) {
                if (!detail::set_future_data_exception(*get_future_data(), ex)) {
                    throw PromiseAlreadySetException{};
                }
            }

        protected:
            struct PromiseData {
                std::shared_ptr<detail::FutureData<T>> _future_data;
            };

            std::shared_ptr<detail::FutureData<T>> get_future_data() const noexcept {
                return _data->_future_data;
            }

            std::shared_ptr<PromiseData> get_data() const noexcept {
                return _data;
            }

            static std::shared_ptr<PromiseData> create_promise_data(std::shared_ptr<Executor> executor, std::shared_ptr<CancellationManager> cancellation_manager) {
                std::shared_ptr<PromiseData> return_value{new PromiseData, [](PromiseData* promise_data) {
                    auto future_data = promise_data->_future_data;
                    std::lock_guard<std::mutex> lock{future_data->_mutex};
                    future_data->_is_promise_destroyed = true;
                    future_data->_condition.notify_all();
                    delete promise_data;
                }};

                return_value->_future_data = std::make_shared<FutureData<T>>();
                return_value->_future_data->_executor = executor;
                return_value->_future_data->_cancellation_manager = cancellation_manager;

                return return_value;
            }

        private:
            std::shared_ptr<PromiseData> _data;
        };

        template <typename Container, typename Function>
        inline auto async_all(std::shared_ptr<Executor> executor, std::shared_ptr<CancellationManager> cancellation_data, Container&& container, Function&& function)
            -> CompositeFuture<decltype(function(*container.begin()))>;
        template <typename Container, typename Function>
        inline auto async_all(std::shared_ptr<Executor> executor, std::shared_ptr<CancellationManager> cancellation_data, Container&& container, Function&& function)
            -> CompositeFuture<decltype(function(*container.begin(), support::CancellationDelegateSubscriber{}))>;
    }  // namespace detail

    template <typename T>
    class Future : public detail::FutureBase<T> {
    public:
        using detail::FutureBase<T>::FutureBase;

        T get() {
            detail::FutureBase<T>::get();
            return detail::get_future_data_value(*(this->get_data()));
        }

        template <typename Function>
        auto then(Function&& function) -> support::Future<typename detail::async_api_traits<decltype(function(get()))>::ReturnType> {
            detail::FutureBase<T>::throw_if_invalid();
            using ReturnType = decltype(function(get()));
            return detail::FutureBase<T>::then((std::function<ReturnType(void)>) [function = std::forward<Function>(function), future = *this]() mutable {
                return function(future.get());
            });
        }

        template <typename Function>
        auto then(Function&& function) -> support::Future<typename detail::async_api_traits<decltype(function(get(), CancellationDelegateSubscriber{}))>::ReturnType> {
            detail::FutureBase<T>::throw_if_invalid();
            using ReturnType = decltype(function(get(), CancellationDelegateSubscriber{}));
            auto cancellation_data = this->get_data()->_cancellation_manager;
            return detail::FutureBase<T>::then((std::function<ReturnType(void)>) [function = std::forward<Function>(function), future = *this, cancellation_data]() mutable {
                return function(future.get(), [cancellation_data](CancellationDelegate cancellation_delegate) {
                    return cancellation_data->subscribe(std::move(cancellation_delegate));
                });
            });
        }

        template <typename Function>
        auto then_foreach(Function&& function) {
            detail::FutureBase<T>::throw_if_invalid();
            auto data = this->get_data();
            return then([data, function = std::forward<Function>(function)](auto container) mutable {
                return detail::async_all(data->_executor, data->_cancellation_manager, container, std::forward<Function>(function));
            });
        }

        template <typename Condition, typename = std::enable_if<boost::fusion::traits::is_sequence<T>::value>>
        Future<T> then_filter(Condition&& condition) {
            detail::FutureBase<T>::throw_if_invalid();
            auto data = this->get_data();
            support::Promise<T> promise{data->_executor, data->_cancellation_manager};
            then([promise, condition] (T container) mutable {
                T filtered_data;
                for (auto&& value : container) {
                    if (condition(value)) {
                        filtered_data.emplace_back(value);
                    }
                }
                promise.set_value(filtered_data);
            });

            return promise.get_future();
        }

        template <typename ItemType, typename BinaryOperation, typename = std::enable_if<boost::fusion::traits::is_sequence<T>::value>>
        Future<ItemType> then_reduce(const ItemType& init_value, BinaryOperation&& binary_operation) {
            detail::FutureBase<T>::throw_if_invalid();
            auto data = this->get_data();
            support::Promise<ItemType> promise{data->_executor, data->_cancellation_manager};
            then([promise, init_value, binary_operation] (T container) mutable {
                ItemType result = init_value;
                for (auto&& value : container) {
                    result = binary_operation(result, value);
                }
                promise.set_value(result);
            });

            return promise.get_future();
        }
    };

    template <>
    class Future<void> : public detail::FutureBase<void> {
    public:
        using FutureBase<void>::FutureBase;

        template <typename Function>
        auto then(Function&& function) -> support::Future<typename detail::async_api_traits<decltype(function())>::ReturnType> {
            this->throw_if_invalid();
            using ReturnType = decltype(function());
            return detail::FutureBase<void>::then((std::function<ReturnType(void)>) std::forward<Function>(function));
        }

        template <typename Function>
        auto then(Function&& function) -> support::Future<typename detail::async_api_traits<decltype(function(CancellationDelegateSubscriber{}))>::ReturnType> {
            this->throw_if_invalid();
            using ReturnType = decltype(function(CancellationDelegateSubscriber{}));
            auto cancellation_data = this->get_data()->_cancellation_manager;
            return detail::FutureBase<void>::then((std::function<ReturnType(void)>) [function = std::forward<Function>(function), future = *this, cancellation_data]() mutable {
                return function([cancellation_data](CancellationDelegate cancellation_delegate) {
                    return cancellation_data->subscribe(std::move(cancellation_delegate));
                });
            });
        }
    };

    template <typename T>
    class CompositeFuture : public detail::CompositeFutureBase<T> {
    public:
        using detail::CompositeFutureBase<T>::CompositeFutureBase;

        std::vector<T> get() {
            std::vector<T> return_value;
            for (auto&& future : this->get_futures()) {
                return_value.emplace_back(future.get());
            }
            return return_value;
        }

        template <typename Function>
        auto then(Function&& function) -> CompositeFuture<decltype(function(get()[0]))> {
            using FunctionReturnType = decltype(function(get()[0]));
            return detail::CompositeFutureBase<T>::template then<std::function<FunctionReturnType(T)>, FunctionReturnType>(function);
        }

        template <typename Function>
        auto then(Function&& function) -> CompositeFuture<decltype(function(get()[0], CancellationDelegateSubscriber{}))> {
            using FunctionReturnType = decltype(function(get()[0], CancellationDelegateSubscriber{}));
            return detail::CompositeFutureBase<T>::template then<std::function<FunctionReturnType(T, CancellationDelegateSubscriber)>, FunctionReturnType>(function);
        }

        Future<std::vector<T>> then_filter(std::function<bool(const T&)> condition) {
            auto futures = this->get_futures();
            support::Promise<std::vector<T>> promise;
            struct FilterData {
                size_t number_of_expected_responses;
                size_t handled_responses = 0;
                std::mutex mutex;
                std::vector<T> result;
            };
            auto filter_data = std::make_shared<FilterData>();
            filter_data->number_of_expected_responses = futures.size();
            for (auto&& future : futures) {
                future.then([promise, filter_data, condition] (const T& value) mutable {
                    std::lock_guard<std::mutex> lock{filter_data->mutex};
                    if (condition(value)) {
                        filter_data->result.push_back(value);
                    }
                    if (++filter_data->handled_responses == filter_data->number_of_expected_responses) {
                        promise.set_value(filter_data->result);
                    }
                });
            }

            return promise.get_future();
        }

        Future<T> then_reduce(const T& init_value, std::function<T(const T&, const T&)> binary_operation) {
            auto futures = this->get_futures();

            support::Promise<T> promise;
            struct ReduceData {
                size_t number_of_expected_responses;
                size_t handled_responses = 0;
                std::mutex mutex;
                T result;
            };
            auto filter_data = std::make_shared<ReduceData>();
            filter_data->number_of_expected_responses = futures.size();
            filter_data->result = init_value;
            for (auto&& future : futures) {
                future.then([promise, filter_data, binary_operation] (const T& value) mutable {
                    std::lock_guard<std::mutex> lock{filter_data->mutex};
                    filter_data->result = binary_operation(filter_data->result, value);
                    if (++filter_data->handled_responses == filter_data->number_of_expected_responses) {
                        promise.set_value(filter_data->result);
                    }
                });
            }

            return promise.get_future();
        }
    };

    template <>
    class CompositeFuture<void> : public detail::CompositeFutureBase<void> {
    public:
        using CompositeFutureBase<void>::CompositeFutureBase;

        template <typename Function>
        auto then(Function&& function) -> CompositeFuture<decltype(function())> {
            using FunctionReturnType = decltype(function());
            return CompositeFutureBase<void>::then<std::function<FunctionReturnType()>, FunctionReturnType>(function);
        }

        template <typename Function>
        auto then(Function&& function) -> CompositeFuture<decltype(function(CancellationDelegateSubscriber{}))> {
            using FunctionReturnType = decltype(function(CancellationDelegateSubscriber{}));
            return CompositeFutureBase<void>::then<std::function<FunctionReturnType(CancellationDelegateSubscriber)>, FunctionReturnType>(function);
        }
    };

    template <typename T>
    class Promise : public detail::PromiseBase<T> {
    public:
        using detail::PromiseBase<T>::PromiseBase;

        void set_value(T value) {
            if (!detail::set_future_data_value(*(this->get_future_data()), std::move(value))) {
                throw PromiseAlreadySetException{};
            }
        }

        std::function<void(const T&)> as_callback() noexcept {
            return [promise = *this](const T& value) mutable {
                promise.set_value(value);
            };
        }
    };

    template <>
    class Promise<void> : public detail::PromiseBase<void> {
    public:
        using detail::PromiseBase<void>::PromiseBase;

        void set_value() {
            if (!detail::set_future_as_done(*get_future_data())) {
                throw PromiseAlreadySetException{};
            }
        }

        std::function<void()> as_callback() noexcept  {
            return [promise = *this]() mutable {
                promise.set_value();
            };
        }
    };

    namespace detail {
        inline Promise<void> schedule(FutureScheduler& scheduler, std::shared_ptr<Executor> executor, std::shared_ptr<CancellationManager> cancellation_manager, std::function<support::Future<void>(void)> function) {
            Promise<void> promise{executor, cancellation_manager};
            scheduler.schedule([function = std::move(function), promise, cancellation_manager, executor]() mutable {
                try {
                    auto nested_future = function();
                    const auto is_executor_different = nested_future.get_executor() != executor;
                    auto is_set = std::make_shared<std::atomic<bool>>(false);
                    auto subscription = cancellation_manager->subscribe([promise, nested_future, is_executor_different, is_set]() mutable {
                        if (is_executor_different) {
                            nested_future.cancel();
                        } else if (!is_set->exchange(true)) {
                            promise.set_exception(FutureCanceledException);
                        }
                    });
                    nested_future.then([promise, subscription, is_set]() mutable {
                        subscription.disable();
                        if (!is_set->exchange(true)) {
                            promise.set_value();
                        }
                    });
                } catch (...) {
                    cancellation_manager->cancel(std::current_exception());
                    promise.set_exception(std::current_exception());
                }
            });
            return promise;
        }

        inline Promise<void> schedule(FutureScheduler& scheduler, std::shared_ptr<Executor> executor, std::shared_ptr<CancellationManager> cancellation_manager, std::function<support::CompositeFuture<void>(void)> function) {
            Promise<void> promise{executor, cancellation_manager};
            scheduler.schedule([function = std::move(function), promise, cancellation_manager, executor]() mutable {
                try {
                    auto nested_future = function();
                    auto is_set = std::make_shared<std::atomic<bool>>(false);
                    const auto is_executor_different = nested_future.get_executor() != executor;
                    auto subscription = cancellation_manager->subscribe([promise, nested_future, is_executor_different, is_set]() mutable {
                        if (is_executor_different) {
                            nested_future.cancel();
                        } else if (!is_set->exchange(true)) {
                            promise.set_exception(FutureCanceledException);
                        }
                    });
                    auto counter = std::make_shared<std::atomic<size_t>>(0);
                    nested_future.then([promise, subscription, nested_future, counter, is_set]() mutable {
                        if (counter->fetch_add(1) == nested_future.get_futures().size() - 1) {
                            subscription.disable();
                            if (!is_set->exchange(true)) {
                                promise.set_value();
                            }
                        }
                    });
                } catch (...) {
                    cancellation_manager->cancel(std::current_exception());
                    promise.set_exception(std::current_exception());
                }
            });
            return promise;
        }

        template <typename T>
        inline Promise<T> schedule(FutureScheduler& scheduler, std::shared_ptr<Executor> executor, std::shared_ptr<CancellationManager> cancellation_manager, std::function<support::Future<T>(void)> function) {
            Promise<decltype(function().get())> promise{executor, cancellation_manager};
            scheduler.schedule([function = std::move(function), promise, cancellation_manager, executor]() mutable {
                try {
                    auto nested_future = function();
                    auto is_set = std::make_shared<std::atomic<bool>>(false);
                    const auto is_executor_different = nested_future.get_executor() != executor;
                    auto subscription = cancellation_manager->subscribe([promise, nested_future, is_executor_different, is_set]() mutable {
                        if (is_executor_different) {
                            nested_future.cancel();
                        } else if (!is_set->exchange(true)) {
                            promise.set_exception(FutureCanceledException);
                        }
                    });
                    nested_future.then([promise, nested_future, subscription, is_set](auto) mutable {
                        subscription.disable();
                        if (!is_set->exchange(true)) {
                            promise.set_value(nested_future.get());
                        }
                    });
                } catch (...) {
                    cancellation_manager->cancel(std::current_exception());
                    promise.set_exception(std::current_exception());
                }
            });
            return promise;
        }

        template <typename T>
        inline Promise<std::vector<T>> schedule(FutureScheduler& scheduler, std::shared_ptr<Executor> executor, std::shared_ptr<CancellationManager> cancellation_manager, std::function<support::CompositeFuture<T>(void)> function) {
            Promise<decltype(function().get())> promise{executor, cancellation_manager};
            scheduler.schedule([function = std::move(function), promise, cancellation_manager, executor]() mutable {
                try {
                    auto nested_future = function();

                    struct SharedData {
                        std::mutex mutex;
                        std::atomic<bool> is_set{false};
                        decltype(function().get()) result;
                    };
                    auto shared_data = std::make_shared<SharedData>();
                    const auto is_executor_different = nested_future.get_executor() != executor;
                    auto subscription = cancellation_manager->subscribe([nested_future, is_executor_different, shared_data, promise]() mutable {
                        if (is_executor_different) {
                            nested_future.cancel();
                        } else if (!shared_data->is_set.exchange(true)) {
                            promise.set_exception(FutureCanceledException);
                        }
                    });
                    nested_future.then([promise, shared_data, subscription, nested_future](auto item_result) mutable {
                        bool is_last = false;
                        {
                            std::lock_guard<std::mutex> lock{shared_data->mutex};
                            shared_data->result.emplace_back(item_result);
                            is_last = shared_data->result.size() == nested_future.get_futures().size();
                        }
                        if (is_last && !shared_data->is_set.exchange(true)) {
                            subscription.disable();
                            promise.set_value(shared_data->result);
                        }
                    });
                } catch (...) {
                    cancellation_manager->cancel(std::current_exception());
                    promise.set_exception(std::current_exception());
                }
            });
            return promise;
        }

        template <typename T>
        inline Promise<T> schedule(FutureScheduler& scheduler, std::shared_ptr<Executor> executor, std::shared_ptr<CancellationManager> cancellation_manager, std::function<T(void)> function) {
            Promise<T> promise{executor, cancellation_manager};
            scheduler.schedule([function = std::move(function), promise]() mutable {
                if (promise.get_cancellation_manager()->is_cancellation_triggered()) {
                    promise.set_exception(promise.get_cancellation_manager()->get_exception());
                } else {
                    try {
                        promise.set_value(function());
                    } catch (...) {
                        promise.get_cancellation_manager()->cancel(std::current_exception());
                        promise.set_exception(std::current_exception());
                    }
                }
            });

            return promise;
        }

        template <>
        inline Promise<void> schedule(FutureScheduler& scheduler, std::shared_ptr<Executor> executor, std::shared_ptr<CancellationManager> cancellation_manager, std::function<void(void)> function) {
            Promise<void> promise{executor, cancellation_manager};
            scheduler.schedule([function = std::move(function), promise]() mutable {
                if (promise.get_cancellation_manager()->is_cancellation_triggered()) {
                    promise.set_exception(promise.get_cancellation_manager()->get_exception());
                } else {
                    try {
                        function();
                        promise.set_value();
                    } catch (...) {
                        promise.get_cancellation_manager()->cancel(std::current_exception());
                        promise.set_exception(std::current_exception());
                    }
                }
            });

            return promise;
        }

        template <typename Function, typename... Ts>
        inline auto async(std::shared_ptr<Executor> executor, std::shared_ptr<CancellationManager> cancellation_data, Function function, Ts... args)
        -> support::Future<typename async_api_traits<decltype(function(args...))>::ReturnType> {
            detail::ExecutorScheduler scheduler(executor);
            return detail::schedule(scheduler, executor, cancellation_data, (std::function<decltype(function(args...))()>) [=]{
                return function(args...);
            }).get_future();
        }

        template <typename Function, typename... Ts>
        inline auto async(std::shared_ptr<Executor> executor, std::shared_ptr<CancellationManager> cancellation_data, Function function, Ts... args)
        -> support::Future<typename async_api_traits<decltype(function(CancellationDelegateSubscriber{}, args...))>::ReturnType> {
            detail::ExecutorScheduler scheduler(executor);
            return detail::schedule(scheduler, executor, cancellation_data, (std::function<decltype(function(support::CancellationDelegateSubscriber{}, args...))()>) [=]{
                return function([cancellation_data](CancellationDelegate cancellation_delegate) {
                    return cancellation_data->subscribe(std::move(cancellation_delegate));
                }, args...);
            }).get_future();
        }

        template <typename Function, typename T>
        inline auto async(std::shared_ptr<Executor> executor, std::shared_ptr<CancellationManager> cancellation_data, Function function, T value)
        -> support::Future<decltype(function(value, CancellationDelegateSubscriber{}))> {
            detail::ExecutorScheduler scheduler(executor);
            return detail::schedule(scheduler, executor, cancellation_data, (std::function<decltype(function(value, support::CancellationDelegateSubscriber{}))()>) [=]{
                return function(value, [cancellation_data](CancellationDelegate cancellation_delegate) {
                    return cancellation_data->subscribe(std::move(cancellation_delegate));
                });
            }).get_future();
        }

        template <typename Container, typename Function>
        inline auto async_all(std::shared_ptr<Executor> executor, std::shared_ptr<CancellationManager> cancellation_data, Container&& container, Function&& function)
        -> CompositeFuture<decltype(function(*container.begin()))> {
            using FunctionReturnType = decltype(function(*container.begin()));
            using FutureVectorType = std::vector<support::Future<FunctionReturnType>>;
            FutureVectorType futures;
            for (auto&& item : container) {
                futures.emplace_back(async(executor, cancellation_data, function, item));
            }
            return CompositeFuture<FunctionReturnType>{futures};
        }

        template <typename Container, typename Function>
        inline auto async_all(std::shared_ptr<Executor> executor, std::shared_ptr<CancellationManager> cancellation_data, Container&& container, Function&& function)
        -> CompositeFuture<decltype(function(*container.begin(), support::CancellationDelegateSubscriber{}))> {
            using FunctionReturnType = decltype(function(*container.begin(), support::CancellationDelegateSubscriber{}));
            using FutureVectorType = std::vector<support::Future<FunctionReturnType>>;
            FutureVectorType futures;
            for (auto&& item : container) {
                futures.emplace_back(async(executor, cancellation_data, function, item));
            }
            return CompositeFuture<FunctionReturnType>{futures};
        }

        template <typename Futures, typename Exception>
        auto wait_any(Futures&& futures, std::function<bool()> condition) {
            if (futures.empty()) {
                throw EmptyFutureListException{};
            }

            while (condition()) {
                auto iter = std::begin(futures);
                for (; iter != std::end(futures); ++iter) {
                    try {
                        (*iter).wait_for(std::chrono::milliseconds(10));
                        break;
                    } catch (...) {}
                }

                if (iter != std::end(futures)) {
                    auto return_value = *iter;
                    futures.erase(iter);
                    return return_value;
                }
            }

            throw Exception{};
        }
    }  // namespace detail

    template <typename Function, typename... Ts>
    auto async(Function function, Ts... args) {
        auto executor = std::make_shared<ThreadPoolExecutor>(ThreadPoolExecutor::ShutdownPolicy::KEEP_RUNNING);
        auto cancellation_data = std::make_shared<detail::CancellationManager>();
        return detail::async(executor, cancellation_data, std::forward<Function>(function), std::forward<Ts>(args)...);
    }

    template <typename Container, typename Function>
    auto async_all(Container&& container, Function&& function) {
        auto executor = std::make_shared<ThreadPoolExecutor>(ThreadPoolExecutor::ShutdownPolicy::KEEP_RUNNING);
        auto cancellation_data = std::make_shared<detail::CancellationManager>();
        return detail::async_all(executor, cancellation_data, std::forward<Container>(container), std::forward<Function>(function));
    }

    template <typename Futures>
    inline void wait_all(Futures&& futures) {
        for (auto&& future : futures) {
            future.wait();
        }
    }

    template <typename Futures>
    inline auto wait_any(Futures&& futures)
    -> typename std::remove_reference<decltype(*futures.begin())>::type {
        return detail::wait_any<Futures, FutureException>(std::forward<Futures>(futures), [](){return true;});
    }

    template <typename Futures, class Duration>
    inline auto wait_any_until(Futures&& futures, Duration duration)
    -> typename std::remove_reference<decltype(*futures.begin())>::type {
        return detail::wait_any<Futures, WaitTimeoutException>(std::forward<Futures>(futures),
                                                               [duration]{return std::chrono::system_clock::now() < duration;});
    }

    template <typename Futures, class Period>
    inline auto wait_any_for(Futures&& futures, Period timeout_duration)
    -> typename std::remove_reference<decltype(*futures.begin())>::type {
        return wait_any_until(std::forward<Futures>(futures), std::chrono::system_clock::now() + timeout_duration);
    }

    template <typename Futures>
    inline void cancel_all(Futures&& futures) {
        for (auto&& future : futures) {
            future.cancel();
        }
    }
}  // namespace support
