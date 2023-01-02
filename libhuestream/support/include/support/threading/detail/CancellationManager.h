/*******************************************************************************
Copyright (C) 2019 Signify Holding
All Rights Reserved.
********************************************************************************/

#pragma once

#include <functional>
#include <utility>

#include "support/signals/SynchronousSignal.h"
#include "support/util/Subscription.h"
#include "support/util/ExceptionUtil.h"

namespace support {
    using CancellationDelegate = std::function<void()>;
    using CancellationDelegateSubscriber = std::function<support::Subscription(CancellationDelegate)>;

    namespace detail {
        class CancellationManager {
        public:
            support::Subscription subscribe(CancellationDelegate cancellation_delegate) {
                {
                    std::lock_guard<std::mutex> lock{_mutex};
                    if (!_is_canceled) {
                        return support::Subscription{_signal.connect(std::move(cancellation_delegate))};
                    }
                }

                cancellation_delegate();
                return support::Subscription{};
            }

            void cancel(std::exception_ptr exception) {
                bool emmit_signal = false;
                {
                    std::lock_guard<std::mutex> lock{_mutex};
                    emmit_signal = false;
                    if (!_is_canceled) {
                        emmit_signal = _is_canceled = true;
                        _exception = exception;
                    }
                }

                if (emmit_signal) {
                    call_and_ignore_exception(_signal);
                }
            }

            bool is_cancellation_triggered() {
                std::lock_guard<std::mutex> lock{_mutex};
                return _is_canceled;
            }

            std::exception_ptr get_exception() const {
                return _exception;
            }

        private:
            std::mutex _mutex;
            SynchronousSignal<> _signal;
            bool _is_canceled;
            std::exception_ptr _exception;
        };
    }  // namespace detail
}  // namespace support