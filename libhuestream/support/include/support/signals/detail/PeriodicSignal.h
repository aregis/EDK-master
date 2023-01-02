/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <memory>
#include <utility>

#include "support/signals/detail/Signal.h"
#include "support/signals/detail/PeriodicSignalScheduler.h"

namespace support {
    namespace detail {

        template<typename Dispatcher, template<typename...> class Operation, typename... Ts>
        class PeriodicSignal : public detail::Signal<Dispatcher, PeriodicSignalScheduler<Operation, Ts...>, Ts...> {
        public:
            using Base = detail::Signal<Dispatcher, PeriodicSignalScheduler<Operation, Ts...>, Ts...>;

            explicit PeriodicSignal(std::chrono::milliseconds interval)
                : Base {std::make_shared<PeriodicSignalScheduler<Operation, Ts...>>(interval, [this](Ts... args)->support::Operation {
                        return Base::dispatch_message(std::forward<Ts>(args)...);
                    })} {}

            PeriodicSignal(std::shared_ptr<Scheduler> scheduler, std::chrono::milliseconds interval)
                    : Base {std::make_shared<PeriodicSignalScheduler<Operation, Ts...>>(std::move(scheduler), interval, [this](Ts... args)->support::Operation {
                return Base::dispatch_message(std::forward<Ts>(args)...);
            })} {}
        };
    }  //  namespace detail
}  //  namespace support
