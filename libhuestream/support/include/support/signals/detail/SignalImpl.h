/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <map>
#include <unordered_map>
#include <mutex>
#include <memory>
#include <utility>
#include <functional>
#include <atomic>

#include "support/util/Operation.h"

namespace support {
    namespace detail {

        template <typename Dispatcher, typename Scheduler, typename... Ts>
        class SignalImpl {
        public:
            using Callback = std::function<void(Ts...)>;
            using CallbackContainer = std::unordered_map<std::shared_ptr<Callback>, std::shared_ptr<Dispatcher>>;

            SignalImpl() : _scheduler{std::make_shared<Scheduler>(
                  [this](Ts... args) -> support::Operation {
                      return dispatch_message(std::forward<Ts>(args)...);
                  })} {}

            explicit SignalImpl(std::shared_ptr<Scheduler> scheduler) : _scheduler{std::move(scheduler)} {}

            std::shared_ptr<Callback> connect(Callback callback) {
                if (callback) {
                    return connect(std::make_shared<Callback>(std::move(callback)));
                } else {
                    return std::shared_ptr<Callback>{};
                }
            }

            std::shared_ptr<Callback> connect(std::shared_ptr<Callback> callback) {
                if (callback) {
                    std::lock_guard<std::mutex> lock{_callbacks_mutex};
                    if (_callbacks.find(callback) == _callbacks.end()) {
                        _callbacks.insert({callback, std::make_shared<Dispatcher>()});
                    }
                }
                return callback;
            }

            void disconnect(std::shared_ptr<Callback> callback) {
                std::shared_ptr<Dispatcher> dispatcher;
                if (callback) {
                    std::lock_guard<std::mutex> lock{_callbacks_mutex};
                    auto iter = _callbacks.find(callback);
                    if (iter != _callbacks.end()) {
                        dispatcher = iter->second;
                        _callbacks.erase(iter);
                    }
                }
                if (dispatcher) {
                    dispatcher->shutdown();
                }
            }

            void enable() {
                _is_enabled = true;
            }

            void disable() {
                _is_enabled = false;
            }

            support::Operation operator()(Ts... args) {
                return _scheduler->post(std::forward<Ts>(args)...);
            }

            bool is_active(std::shared_ptr<Callback> callback) const {
                std::lock_guard<std::mutex> lock{_callbacks_mutex};
                return _callbacks.find(callback) != _callbacks.end() && !_is_shutdown;
            }

            void shutdown() {
                _is_shutdown = true;

                CallbackContainer callbacks = get_callbacks();
                for (auto&& callback : callbacks) {
                    callback.second->shutdown();
                }
            }

            support::Operation dispatch_message(Ts... args) {
                support::CompositeOperation operations;

                if (_is_enabled && !_is_shutdown) {
                    CallbackContainer callbacks = get_callbacks();
                    for (auto&& callback : callbacks) {
                        auto local_callback = (*callback.first);
                        operations.insert(callback.second->post([local_callback, args...]() {
                            local_callback(args...);
                        }));
                    }
                }

                return operations;
            }

        private:
            CallbackContainer get_callbacks() {
                std::lock_guard<std::mutex> lock{_callbacks_mutex};
                return _callbacks;
            }

            CallbackContainer _callbacks;
            mutable std::mutex _callbacks_mutex;
            std::atomic<bool> _is_enabled{true};
            std::atomic<bool> _is_shutdown{false};
            std::shared_ptr<Scheduler> _scheduler;
        };

    }  //  namespace detail
}  // namespace support
