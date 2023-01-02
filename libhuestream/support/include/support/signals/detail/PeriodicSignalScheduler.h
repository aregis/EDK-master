/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <utility>
#include <tuple>
#include <vector>
#include <memory>

#include <boost/optional.hpp>

#include "support/util/Operation.h"
#include "support/scheduler/Scheduler.h"
#include "support/threading/ConditionVariable.h"
#include "support/util/TupleUtil.h"

namespace support {
    namespace detail {

        template <typename Scheduler, template<typename...> class Operator, typename... Ts>
        class PeriodicSignalSchedulerImpl {
            class SchedulerOperation;

        public:
            using Trigger = std::function<support::Operation(Ts...)>;

            PeriodicSignalSchedulerImpl(std::chrono::milliseconds interval, Trigger trigger)
                    : PeriodicSignalSchedulerImpl(std::make_shared<Scheduler>(static_cast<unsigned int>(interval.count())), interval, std::move(trigger)) {
                _scheduler->start();
                _own_scheduler = true;
            }

            PeriodicSignalSchedulerImpl(std::shared_ptr<Scheduler> scheduler, std::chrono::milliseconds interval, Trigger trigger)
                    : _trigger{std::move(trigger)}
                    , _scheduler_task{static_cast<int>(reinterpret_cast<int64_t>(this)), static_cast<unsigned int>(interval.count()), std::bind(&PeriodicSignalSchedulerImpl::dispatch, this)}
                    , _scheduler{std::move(scheduler)} {
            }

            virtual ~PeriodicSignalSchedulerImpl() {
                if (_own_scheduler) {
                    _scheduler->stop();
                }
                dispatch();
            }

            virtual support::Operation post(Ts... args) {
                std::lock_guard<std::mutex> lock{_mutex};

                if (_operation == nullptr || _operation->is_canceled()) {
                    _operation = std::make_shared<SchedulerOperation>();
                    if (!_task_scheduled) {
                        _scheduler->add_task(_scheduler_task);
                        _task_scheduled = true;
                    }
                }

                _operation->add_data(std::forward<Ts>(args)...);

                return support::Operation{_operation};
            }

        private:
            void dispatch() {
                std::lock_guard<std::mutex> lock{_mutex};

                if (_operation != nullptr && !_operation->is_canceled()) {
                    support::CompositeOperation dispatch_operation;
                    auto data = _operation->get_data();
                    for (auto&& item : data) {
                        dispatch_operation.insert(support::apply(_trigger, item));
                    }

                    _operation->set_triggered(std::move(dispatch_operation));
                }

                if (_task_scheduled) {
                    _scheduler->remove_task(_scheduler_task.get_id());
                    _task_scheduled = false;
                }

                _operation = {};
            }

            class SchedulerOperation : public support::IOperation {
            public:
                SchedulerOperation() {}

                void add_data(Ts... args) {
                    _operator(std::forward<Ts>(args)...);
                }

                std::vector<std::tuple<Ts...>> get_data() const {
                    return to_vector(_operator.get());
                }

                void set_triggered(support::CompositeOperation dispatch_operation) {
                    _dispatch_operation = std::move(dispatch_operation);
                    _is_triggered.set_value(true);
                }

                void wait() override {
                    _is_triggered.wait(true);
                    _dispatch_operation.wait();
                }

                void cancel() override {
                    _is_canceled = true;
                }

                bool is_canceled() const {
                    return _is_canceled;
                }

                bool is_cancelable() const override {
                    return true;
                }

            private:
                std::vector<std::tuple<Ts...>> to_vector(const boost::optional<std::tuple<Ts...>>& item) const {
                    std::vector<std::tuple<Ts...>> return_value;
                    if (item != boost::none) {
                        return_value.push_back(*item);
                    }
                    return return_value;
                }

                std::vector<std::tuple<Ts...>> to_vector(std::vector<std::tuple<Ts...>> item) const {
                    return item;
                }

                Operator<Ts...> _operator;
                support::CompositeOperation _dispatch_operation;
                support::ConditionVariable<bool> _is_triggered{false};
                std::atomic<bool> _is_canceled{false};
            };

            std::mutex _mutex;
            Trigger _trigger;
            bool _task_scheduled = false;
            bool _own_scheduler = false;
            support::SchedulerTask _scheduler_task;
            std::shared_ptr<Scheduler> _scheduler;
            std::shared_ptr<SchedulerOperation> _operation;
        };
    }  //  namespace detail

    template <template<typename...> class Aggregator, typename... Ts>
    using PeriodicSignalScheduler = detail::PeriodicSignalSchedulerImpl<Scheduler, Aggregator, Ts...>;

}  // namespace support