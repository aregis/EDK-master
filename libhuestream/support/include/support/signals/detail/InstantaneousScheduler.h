/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <utility>
#include <functional>

#include "support/util/Operation.h"

namespace support {
    namespace detail {

        template <typename... Ts>
        class InstantaneousScheduler {
        public:
            using Trigger = std::function<support::Operation(Ts...)>;

            explicit InstantaneousScheduler(Trigger trigger) : _trigger(std::move(trigger)) {}

            virtual support::Operation post(Ts... args) {
                return _trigger(std::forward<Ts>(args)...);
            }

            virtual ~InstantaneousScheduler() = default;

        private:
            Trigger _trigger;
        };

    }  //  namespace detail
}  // namespace support