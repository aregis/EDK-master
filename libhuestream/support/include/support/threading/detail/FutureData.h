/*******************************************************************************
Copyright (C) 2019 Signify Holding
All Rights Reserved.
********************************************************************************/

#pragma once

#include <functional>
#include <memory>
#include <utility>
#include <vector>

#include "support/threading/ThreadPoolExecutor.h"
#include "support/threading/detail/CancellationManager.h"

namespace support {
    namespace detail {
        template <typename T>
        struct ValueHolder {
            std::unique_ptr<T> _value;
        };

        template <>
        struct ValueHolder<void> {};

        template <typename T>
        struct FutureData {
            std::mutex _mutex;
            std::condition_variable _condition;
            std::exception_ptr _exception;
            std::atomic<bool> _is_set{false};
            bool _is_promise_destroyed = false;
            std::vector<std::function<void()>> _continuations;
            std::shared_ptr<Executor> _executor;
            std::shared_ptr<CancellationManager> _cancellation_manager;
            ValueHolder<T> _value_holder;
        };

        template <typename T>
        inline bool set_future_as_done(FutureData<T>& future_data) {
            bool return_value = false;

            std::vector<std::function<void()>> continuations;
            {
                std::lock_guard<std::mutex> lock{future_data._mutex};
                return_value = !future_data._is_set;
                future_data._is_set = true;
                future_data._condition.notify_all();
                continuations.swap(future_data._continuations);
            }

            if (return_value) {
                for (auto&& continuation : continuations) {
                    continuation();
                }
            }

            return return_value;
        }

        template <typename T>
        inline bool set_future_data_value(FutureData<T>& future_data, T value) {
            {
                std::lock_guard<std::mutex> lock{future_data._mutex};
                future_data._value_holder._value = std::make_unique<T>(std::move(value));
            }

            return set_future_as_done(future_data);
        }

        template <typename T>
        inline bool set_future_data_exception(FutureData<T>& future_data, std::exception_ptr exception) {
            {
                std::lock_guard<std::mutex> lock{future_data._mutex};
                future_data._exception = std::move(exception);
                future_data._value_holder = ValueHolder<T>{};
            }

            return set_future_as_done(future_data);
        }

        template <typename T>
        inline T get_future_data_value(FutureData<T>& future_data) {
            return *future_data._value_holder._value;
        }

        template <>
        inline void get_future_data_value(FutureData<void>&) {}
    }  // namespace detail
}  // namespace support