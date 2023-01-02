/*******************************************************************************
Copyright (C) 2019 Signify Holding
All Rights Reserved.
********************************************************************************/

#include <algorithm>
#include <functional>

#include "support/threading/OperationalQueue.h"
#include "support/threading/ThreadPool.h"
#include "support/logging/Log.h"

namespace support {

    OperationalQueue::OperationalQueue()
            : OperationalQueue(GlobalThreadPool::get()) {
    }

    OperationalQueue::OperationalQueue(std::shared_ptr<support::ThreadPool> thread_pool)
            : _state(std::make_shared<State>()) {
        _state->thread_pool = std::move(thread_pool);
    }

    OperationalQueue::~OperationalQueue() {
        stop_processing();
    }

    OperationalQueue::TicketHandle OperationalQueue::create_ticket(std::function<void()> invocable) {
        auto descriptor = std::make_shared<TicketDescriptor>();
        descriptor->handle = std::make_shared<_TicketHandle>();
        descriptor->invocable = std::move(invocable);
        descriptor->completed_future = descriptor->completed_promise.get_future().share();

        std::lock_guard<std::mutex> lock(_state->sync_mutex);
        if (_state->stop_processing_requested) {
            return TicketHandle();
        }

        _state->registry[descriptor->handle] = descriptor;
        return descriptor->handle;
    }

    void OperationalQueue::schedule_ticket(const OperationalQueue::TicketHandle& ticket) {
        bool schedule = false;
        {
            std::lock_guard<std::mutex> lock(_state->sync_mutex);
            if (_state->stop_processing_requested) {
                return;
            }

            auto it = _state->registry.find(ticket);
            if (it == _state->registry.end()) {
                return;
            }

            if (it->second->was_scheduled) {
                return;
            }

            if (!_state->is_scheduled) {
                schedule = _state->is_scheduled = true;
            }

            it->second->was_scheduled = true;

            _state->processing_queue.push_back(it->second);
            _state->sync_condition.notify_all();
        }

        if (schedule) {
            schedule_processing_loop();
        }
    }

    void OperationalQueue::schedule_processing_loop() {
        _state->thread_pool->add_task([state = _state] () {
            {
                std::lock_guard<std::mutex> lock(state->sync_mutex);
                state->processing_thread_id = std::this_thread::get_id();
            }

            while (true) {
                auto is_done = [state] {
                    return state->processing_queue.empty() || state->stop_processing_requested;
                };

                processing_loop(state, [state, is_done] {
                    std::lock_guard<std::mutex> lock(state->sync_mutex);
                    return is_done();
                });

                std::lock_guard<std::mutex> lock(state->sync_mutex);
                state->is_scheduled = !is_done();
                if (!state->is_scheduled) {
                    state->processing_thread_id = {};
                    state->idle_condition.notify_all();
                    break;
                }
            }
        });
    }

    void OperationalQueue::discard_ticket(const OperationalQueue::TicketHandle& ticket) {
        bool is_currently_running = false;
        TicketDescriptorPtr removed_ticket;

        {
            std::lock_guard<std::mutex> lock(_state->sync_mutex);

            auto it = _state->registry.find(ticket);
            if (it == _state->registry.end())
                return;

            auto current_it = std::find_if(std::begin(_state->current_ticket_stack), std::end(_state->current_ticket_stack), [&ticket] (const TicketDescriptorPtr& d) {
                return ticket == d->handle;
            });

            if (current_it != std::end(_state->current_ticket_stack)) {
                is_currently_running = true;
            } else {
                _state->processing_queue.remove_if([&it](const TicketDescriptorPtr& d) {
                    return it->second->handle == d->handle;
                });

                removed_ticket = it->second;
                _state->registry.erase(it);
            }
        }

        if (removed_ticket) {
            removed_ticket->completed_promise.set_value();
        }

        if (is_currently_running) {
            wait_ticket(ticket);
        }
    }

    void OperationalQueue::wait_ticket(const OperationalQueue::TicketHandle& ticket) {
        const TicketDescriptorPtr descriptor = get_descriptor(ticket);

        if (descriptor) {
            const bool recursive_wait = this_thread_is_processing_thread();

            if (!recursive_wait) {
                descriptor->completed_future.wait();
            } else if (!is_in_current_ticket_stack(descriptor)) {
                processing_loop(_state, [this, ticket] {
                    return !has_ticket(ticket);
                });
            } else {
                // Ignore
            }
        }
    }

    bool OperationalQueue::this_thread_is_processing_thread() {
        std::lock_guard<std::mutex> lock(_state->sync_mutex);
        return std::this_thread::get_id() == _state->processing_thread_id;
    }

    bool OperationalQueue::is_in_current_ticket_stack(TicketDescriptorPtr descriptor) {
        std::lock_guard<std::mutex> lock(_state->sync_mutex);

        auto current_it = std::find_if(std::begin(_state->current_ticket_stack),
                                       std::end(_state->current_ticket_stack),
                                       [&descriptor](const TicketDescriptorPtr& d) {
                                           return descriptor->handle == d->handle;
                                       });

        return current_it != std::end(_state->current_ticket_stack);
    }

    OperationalQueue::TicketDescriptorPtr OperationalQueue::get_descriptor(const OperationalQueue::TicketHandle& ticket) {
        std::lock_guard<std::mutex> lock(_state->sync_mutex);

        auto it = _state->registry.find(ticket);
        if (it == _state->registry.end()) {
            return nullptr;
        } else {
            return it->second;
        }
    }

    void OperationalQueue::processing_loop(std::shared_ptr<State> state, std::function<bool()> recursion_break_condition) {
        while (!recursion_break_condition()) {
            TicketDescriptorPtr current_ticket;

            {
                std::unique_lock<std::mutex> lock(state->sync_mutex);
                state->sync_condition.wait(lock, [state] {
                    return (!state->processing_queue.empty()
                            || state->stop_processing_requested);
                });

                if (state->stop_processing_requested) {
                    break;
                }

                current_ticket = state->processing_queue.front();
                state->current_ticket_stack.push_back(current_ticket);
                state->processing_queue.pop_front();
            }

            call_and_ignore_exception(current_ticket->invocable);
            state->thread_pool->process_tick();

            {
                std::lock_guard<std::mutex> lock(state->sync_mutex);
                state->current_ticket_stack.pop_back();
                state->registry.erase(current_ticket->handle);
            }

            current_ticket->invocable = {};
            current_ticket->completed_promise.set_value();
        }
    }

    bool OperationalQueue::run_recursive_process_tickets_loop(const std::function<bool()>& break_condition) {
        if (!this_thread_is_processing_thread()) {
            return false;
        }

        processing_loop(_state, break_condition);
        return true;
    }

    void OperationalQueue::stop_processing() {
        std::unique_lock<std::mutex> lock(_state->sync_mutex);
        if (_state->stop_processing_requested) {
            return;
        }

        _state->stop_processing_requested = true;
        _state->sync_condition.notify_all();

        _state->idle_condition.wait(lock, [state = _state]{ return !state->is_scheduled; });
    }

    bool OperationalQueue::has_ticket(const OperationalQueue::TicketHandle &ticket) {
        std::lock_guard<std::mutex> lock(_state->sync_mutex);

        auto it = _state->registry.find(ticket);
        return it != _state->registry.end();
    }

    size_t OperationalQueue::ticket_count() {
        std::lock_guard<std::mutex> lock(_state->sync_mutex);
        return _state->registry.size();
    }

    std::shared_ptr<ThreadPool> OperationalQueue::get_thread_pool() const {
        return _state->thread_pool;
    }

}  // namespace support
