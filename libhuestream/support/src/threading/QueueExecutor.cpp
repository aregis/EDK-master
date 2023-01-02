/*******************************************************************************
Copyright (C) 2019 Signify Holding
All Rights Reserved.
********************************************************************************/

#include <algorithm>
#include <list>
#include <mutex>
#include <utility>

#include "support/threading/QueueExecutor.h"

#include "support/threading/OperationalQueue.h"
#include "support/threading/detail/TaskSchedule.h"
#include "support/threading/ThreadPool.h"

using support::Operation;

namespace support {

    struct QueueExecutor::Impl {
        struct TicketInfo {
            OperationalQueue::TicketHandle ticket;
            bool wait_on_cleanup;
        };

        std::list<TicketInfo> tickets;
        std::shared_ptr<OperationalQueue> queue;
        std::mutex sync;
        support::detail::TaskSchedule task_schedule;
        bool closing;
        support::Subscription task_execute_subscription;

        explicit Impl(std::shared_ptr<OperationalQueue> i_queue)
                : queue(i_queue)
                , closing(false) {
        }
    };

    class QueueExecutorOperation : public support::IOperation {
    public:
        QueueExecutorOperation() = default;
        QueueExecutorOperation(std::weak_ptr<OperationalQueue> queue,
                               std::weak_ptr<support::OperationalQueue::_TicketHandle> ticket_handle,
                               QueueExecutor::OperationType operation_type)
            : _queue{queue}
            , _ticket_handle{ticket_handle}
            , _operation_type{operation_type} {}

        void wait() override {
            auto queue = _queue.lock();
            auto ticket_handle = _ticket_handle.lock();
            if (queue && ticket_handle) {
                queue->wait_ticket(ticket_handle);
            }
        }

        bool is_cancelable() const override {
            return QueueExecutor::OperationType::CANCELABLE == _operation_type;
        }

        void cancel() override {
            auto queue = _queue.lock();
            auto ticket_handle = _ticket_handle.lock();
            if (queue && ticket_handle) {
                queue->discard_ticket(ticket_handle);
            }
        }

    private:
        std::weak_ptr<OperationalQueue> _queue;
        std::weak_ptr<OperationalQueue::_TicketHandle> _ticket_handle;
        QueueExecutor::OperationType _operation_type{ QueueExecutor::OperationType::CANCELABLE };
    };

    QueueExecutor::QueueExecutor()
            : QueueExecutor{GlobalQueueExecutor::get()->get_operational_queue()} {}

    QueueExecutor::QueueExecutor(std::shared_ptr<OperationalQueue> queue)
            : _impl(std::make_shared<Impl>(queue)) {}

    QueueExecutor::~QueueExecutor() {
        shutdown();
    }

    void QueueExecutor::shutdown() {
        support::Subscription task_execute_subscription;
        {
            std::lock_guard<decltype(_impl->sync)> lock(_impl->sync);
            task_execute_subscription = std::move(_impl->task_execute_subscription);
            _impl->closing = true;
        }
        task_execute_subscription = {};

        clear(OperationType::CANCELABLE);
    }

    void QueueExecutor::wait_all() {
        clear(OperationType::NON_CANCELABLE);
    }

    void QueueExecutor::cancel_all() {
        clear(OperationType::CANCELABLE);
    }

    support::Operation QueueExecutor::execute(std::function<void()> invocable, OperationType operation_type /* = OperationType::CANCELABLE */) {
        {
            std::lock_guard<std::mutex> lock(_impl->sync);
            if (_impl->closing) {
                return Operation{std::make_shared<QueueExecutorOperation>()};
            }
        }

        std::weak_ptr<Impl> weak_impl = _impl;
        auto this_ticket = std::make_shared<OperationalQueue::TicketHandle>();
        auto ticket = _impl->queue->create_ticket([invocable, weak_impl, this_ticket] {
            invocable();

            auto impl = weak_impl.lock();
            if (impl) {
                std::lock_guard<decltype(impl->sync)> lock(impl->sync);
                impl->tickets.remove_if([this_ticket](const Impl::TicketInfo& info){
                    return info.ticket == *this_ticket;
                });
            }
        });

        *this_ticket = ticket;

        bool posted = false;

        do {
            std::lock_guard<std::mutex> lock(_impl->sync);
            if (_impl->closing) {
                break;
            }

            _impl->tickets.push_back(Impl::TicketInfo{ticket, operation_type == OperationType::NON_CANCELABLE});
            _impl->queue->schedule_ticket(ticket);
            posted = true;
        } while (false);

        if (!posted) {
            _impl->queue->discard_ticket(ticket);
        }

        return Operation{std::make_shared<QueueExecutorOperation>(_impl->queue, ticket, operation_type)};
    }

    void QueueExecutor::clear(QueueExecutor::OperationType policy) {
        std::list<Impl::TicketInfo> tickets;
        std::list<OperationalQueue::TicketHandle> closed_tickets;

        do {
            {
                std::lock_guard<std::mutex> lock(_impl->sync);
                _impl->tickets.remove_if([this](const Impl::TicketInfo& info) {
                    return !_impl->queue->has_ticket(info.ticket);
                });

                tickets = _impl->tickets;
            }

            // If `clear` is called from the same queue, which is processing the tickets,
            // call to `discard_ticket` or `wait_ticket` will not remove the ticket from the queue.
            // Note, that `_impl->tickets` should still contain those "incomplete" tickets. In this way, calls to
            // `QueueExecutor::clear` will be correctly handled in case of being invoked from any other thread.

            tickets.remove_if([&closed_tickets](const Impl::TicketInfo& info) {
                return std::find(std::begin(closed_tickets), std::end(closed_tickets), info.ticket) != std::end(closed_tickets);
            });

            // reverse order to discard tickets that are not yet posted since we are not blocking queue here
            for (auto it = tickets.rbegin(); it != tickets.rend(); ++it) {
                if (!it->wait_on_cleanup && policy == OperationType::CANCELABLE) {
                    _impl->queue->discard_ticket(it->ticket);
                    closed_tickets.push_back(it->ticket);
                }
            }

            // forward order for wait
            for (auto it = std::begin(tickets); it != std::end(tickets); ++it) {
                if (it->wait_on_cleanup || policy == OperationType::NON_CANCELABLE) {
                    _impl->queue->wait_ticket(it->ticket);
                    closed_tickets.push_back(it->ticket);
                }
            }

            // pickup everything that may come in during cleanup
        } while (!tickets.empty());
    }

    std::shared_ptr<OperationalQueue> QueueExecutor::get_operational_queue() const {
        return _impl->queue;
    }

    void QueueExecutor::schedule(std::chrono::steady_clock::time_point time_point, std::function<void()> invocable, OperationType operation_type /* = OperationType::CANCELABLE */) {
        std::lock_guard<std::mutex> lock(_impl->sync);
        _impl->task_schedule.add_task(std::move(time_point), std::move(invocable), operation_type == OperationType::CANCELABLE);

        if (_impl->task_execute_subscription || _impl->closing) return;

        _impl->task_execute_subscription = support::Subscription{_impl->queue->get_thread_pool()->subscribe_for_tick_events([this]() {
            support::detail::TaskSchedule::TaskContainer tasks;
            {
                std::lock_guard<std::mutex> lock(_impl->sync);
                tasks = _impl->task_schedule.filter_and_erase_tasks(std::chrono::steady_clock::now());
            }
            for (auto&& task : tasks) {
                this->execute(task.first, task.second ? OperationType::CANCELABLE : OperationType::NON_CANCELABLE);
            }
        })};
    }
}  // namespace support
