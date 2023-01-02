/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <functional>
#include <memory>
#include <mutex>

#include "support/util/Subscription.h"
#include "support/signals/detail/SignalImpl.h"

namespace support {
    namespace detail {

        template <typename Dispatcher, typename Scheduler, typename... Ts>
        class ConnectionImpl : public support::ISubscription {
        public:
            using Callback = std::function<void(Ts...)>;

            ConnectionImpl(std::shared_ptr<SignalImpl<Dispatcher, Scheduler, Ts...>> signal_impl, std::shared_ptr<Callback> callback) {
                _signal_impl = signal_impl;
                _callback = callback;
            }

            ~ConnectionImpl() {
                disable();
            }

            void enable() override {
                auto signal_impl = _signal_impl.lock();
                if (signal_impl) {
                    signal_impl->connect(_callback);
                }
            }

            void disable() override {
                auto signal_impl = _signal_impl.lock();
                if (signal_impl) {
                    signal_impl->disconnect(_callback);
                }
            }

            bool is_active() const {
                auto signal_impl = _signal_impl.lock();
                return signal_impl ? signal_impl->is_active(_callback) : false;
            }

        private:
            std::weak_ptr<SignalImpl<Dispatcher, Scheduler, Ts...>> _signal_impl;
            std::shared_ptr<Callback> _callback;
        };

    }  //  namespace detail
}  // namespace support

