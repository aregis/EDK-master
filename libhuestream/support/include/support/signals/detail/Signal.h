/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <memory>
#include <utility>
#include <functional>

#include "support/util/Operation.h"
#include "support/util/Subscription.h"

#include "support/signals/detail/InstantaneousScheduler.h"
#include "support/signals/detail/SignalImpl.h"
#include "support/signals/detail/ConnectionImpl.h"

namespace support {
    namespace detail {

        template <typename Dispatcher, typename Scheduler, typename... Ts>
        class Signal {
        public:
            using Callback = std::function<void(Ts...)>;

            Signal() : _impl{std::make_shared<detail::SignalImpl<Dispatcher, Scheduler, Ts...>>()}{};
            explicit Signal(std::shared_ptr<Scheduler> scheduler)
                    : _impl{std::make_shared<detail::SignalImpl<Dispatcher, Scheduler, Ts...>>(std::move(scheduler))}{};

            Signal(const Signal&) = delete;
            Signal& operator=(const Signal&) = delete;

            Signal(Signal&&) = default;
            Signal& operator=(Signal&&) = default;

            ~Signal() {
                if (_impl) {
                    _impl->shutdown();
                }
            }

            Subscription connect(Callback callback) {
                return Subscription{std::make_shared<ConnectionImpl<Dispatcher, Scheduler, Ts...>>(_impl, _impl->connect(std::move(callback)))};
            }

            void enable() {
                _impl->enable();
            }

            void disable() {
                _impl->disable();
            }

            support::Operation operator()(Ts... args) {
                return (*_impl)(args...);
            }

            Scheduler& get_scheduler() {
                return _impl->get_scheduler();
            }

            void shutdown() {
                _impl->shutdown();
            }

        protected:
            support::Operation dispatch_message(Ts... args) {
                return _impl->dispatch_message(std::forward<Ts>(args)...);
            }

        private:
            std::shared_ptr<detail::SignalImpl<Dispatcher, Scheduler, Ts...>> _impl;
        };

    }  //  namespace detail
}  // namespace support
