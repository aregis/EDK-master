/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <mutex>
#include <functional>
#include <condition_variable>
#include <utility>
#include <vector>

namespace support {

    enum class NotifyMode {
        All,
        One
    };

    template <typename T>
    class ConditionVariable {
    public:
        explicit ConditionVariable(const T& value) : _value(value) {}

        void set_value(const T& value, NotifyMode mode = NotifyMode::All) {
            perform([&](T&){_value = value;}, mode);
        }

        T get_value() const {
            std::lock_guard<std::mutex> lock{_mutex};
            return _value;
        }

        void perform(std::function<void(T&)> operation, NotifyMode mode = NotifyMode::All) {
            std::vector<std::function<void()>> handlers;

            {
                std::lock_guard<std::mutex> lock{_mutex};
                operation(_value);
                handlers = take_fulfilled_expectations_handlers(mode);

                if (mode == NotifyMode::All) {
                    _condition.notify_all();
                } else if (handlers.size() == 0) {
                    _condition.notify_one();
                }
            }

            for (auto&& handler : handlers) {
                handler();
            }
        }

        void wait(const T& value) {
            std::unique_lock<std::mutex> lock{_mutex};
            _condition.wait(lock, [&]() {return _value == value;});
        }

        void wait(std::function<bool(const T&)> matcher) {
            std::unique_lock<std::mutex> lock{_mutex};
            _condition.wait(lock, [&]() {return matcher(_value);});
        }

        template< class Rep, class Period>
        bool wait_for(const T& value, const std::chrono::duration<Rep, Period>& rel_time) {
            std::unique_lock<std::mutex> lock{_mutex};
            return _condition.wait_for(lock, rel_time, [&]() {return value == _value;});
        }

        template< class Rep, class Period>
        bool wait_for(std::function<bool(const T&)> matcher, const std::chrono::duration<Rep, Period>& rel_time) {
            std::unique_lock<std::mutex> lock{_mutex};
            return _condition.wait_for(lock, rel_time, [&]() {return matcher(_value);});
        }

        template< class Clock, class Duration >
        bool wait_until(const T& value, const std::chrono::time_point<Clock, Duration>& timeout_time) {
            std::unique_lock<std::mutex> lock{_mutex};
            return _condition.wait_until(lock, timeout_time, [&]() {return value == _value;});
        }

        template< class Clock, class Duration >
        bool wait_until(std::function<bool(const T&)> matcher, const std::chrono::time_point<Clock, Duration>& timeout_time) {
            std::unique_lock<std::mutex> lock{_mutex};
            return _condition.wait_until(lock, timeout_time, [&]() {return matcher(_value);});
        }

        class ConditionBuilder {
        public:
            ConditionBuilder(std::function<bool(const T&)> matcher, ConditionVariable<T>& condition_variable)
                : _matcher{std::move(matcher)}
                , _condition_variable{condition_variable} {}
            ConditionBuilder& then(std::function<void()> handler) {
                bool call_handler = false;
                {
                    std::lock_guard<std::mutex> lock{_condition_variable._mutex};
                    if (_matcher(_condition_variable._value)) {
                        call_handler = true;
                    } else {
                        _condition_variable._expectations.push_back(std::make_pair(_matcher, std::move(handler)));
                    }
                }

                if (call_handler) {
                    handler();
                }

                return *this;
            }

        private:
            std::function<bool(const T&)> _matcher;
            ConditionVariable<T>& _condition_variable;
        };

        ConditionBuilder when(std::function<bool(const T&)> matcher) {
            return ConditionBuilder{std::move(matcher), *this};
        }

    private:
        std::vector<std::function<void()>> take_fulfilled_expectations_handlers(NotifyMode mode) {
            std::vector<std::function<void()>> return_value;
            auto iter = _expectations.begin();
            while (iter != _expectations.end()) {
                if ((*iter).first(_value)) {
                    return_value.push_back((*iter).second);
                    iter = _expectations.erase(iter);
                    if (mode == NotifyMode::One) break;
                } else {
                    ++iter;
                }
            }
            return return_value;
        }

        mutable std::mutex _mutex;
        T _value;
        std::condition_variable _condition;
        std::vector<std::pair<std::function<bool(const T&)>, std::function<void()>>> _expectations;
    };

    namespace operations {
        template <typename T>
        void increment(T& value) {
            ++value;
        }

        template <typename T>
        void decrement(T& value) {
            --value;
        }

        template <typename T>
        std::function<void(T&)> add(const T& value) {
            return std::bind([](T& member, const T& value){member += value;}, std::placeholders::_1, value);
        }

        template <typename T>
        std::function<void(T&)> subtract(const T& value) {
            return std::bind([](T& member, const T& value){member -= value;}, std::placeholders::_1, value);
        }
    }  //  namespace operations

    namespace matchers {
        template <typename T>
        std::function<bool(const T&)> equal(const T& value) {
            return std::bind([](const T& member, const T& value){return member == value;}, std::placeholders::_1, value);
        }

        template <typename T>
        std::function<bool(const T&)> greater_than(const T& value) {
            return std::bind([](const T& member, const T& value){return member > value;}, std::placeholders::_1, value);
        }

        template <typename T>
        std::function<bool(const T&)> less_than(const T& value) {
            return std::bind([](const T& member, const T& value){return member < value;}, std::placeholders::_1, value);
        }
    }  //  namespace matchers

}  //  namespace support
