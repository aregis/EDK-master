/*******************************************************************************
Copyright (C) 2019 Signify Holding
All Rights Reserved.
********************************************************************************/
#pragma once

#include <condition_variable>
#include <functional>
#include <future>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

#include "support/signals/SynchronousSignal.h"

namespace support {
    class ThreadPool;

    /**
     * Sequential queue.
     *
     * Tickets are executed in the order that they are scheduled. At most one ticket is executing
     * at any any time. However, when a ticket waits (with `wait_ticket`) for another ticket, the
     * waiting ticket is effectively suspended and scheduled tickets are executed until the awaited
     * ticket has completed, after which the waiting ticket continues.
     *
     * For example, given scheduled tickets A, B, C, D, where A at some points awaits C (at the *),
     * the execution order on the single processing thread is:
     *
     * |---A-* |---B---| |---C---| *-A---| |---D---|
     */
    class OperationalQueue {
    public:
        OperationalQueue();
        explicit OperationalQueue(std::shared_ptr<ThreadPool> thread_pool);

        /**
         * Waits for all executing tickets to complete.
         */
        virtual ~OperationalQueue();

        OperationalQueue(const OperationalQueue&) = delete;
        OperationalQueue& operator=(const OperationalQueue&) = delete;

        struct _TicketHandle {};
        using TicketHandle = std::shared_ptr<_TicketHandle>;

        TicketHandle create_ticket(std::function<void()> invocable);

        /**
         * Schedule ticket to be executed after all previously scheduled tickets have completed.
         */
        void schedule_ticket(const TicketHandle& ticket);

        /**
         * Delete a (possibly scheduled) ticket or wait for the ticket to complete when it
         * has started executing.
         */
        void discard_ticket(const TicketHandle& ticket);

        /**
         * Wait for the ticket to (be scheduled and) complete or to be discarded.
         *
         * When called from a running ticket, it will effectively suspend that ticket and
         * continue executing scheduled tickets till the awaited ticket has completed (or
         * has been discarded).
         */
        void wait_ticket(const TicketHandle& ticket);

        /**
         * @return Whether the handle refers to a ticket has been created by this OperationalQueue
         * and has not completed executing nor have been discarded.
         */
        bool has_ticket(const TicketHandle& ticket);

        /**
         * @return Number of created tickets that haven't completed executing and haven't been discarded.
         */
        size_t ticket_count();

        /**
         * When executed from a ticket, execute scheduled tickets break_condition returns true;
         * Otherwise, do nothing.
         * @return Whether executed from a ticket
         */
        bool run_recursive_process_tickets_loop(const std::function<bool()>& break_condition);

        /**
         * @return Whether the current tread is the processing thread
         */
        bool this_thread_is_processing_thread();

        /**
         * @return Thread pool from which this operational queue borrows thread
         */
        std::shared_ptr<ThreadPool> get_thread_pool() const;

    protected:
        struct TicketDescriptor {
            TicketHandle handle;
            std::function<void()> invocable;
            std::promise<void> completed_promise;
            std::shared_future<void> completed_future;
            bool was_scheduled = false;
        };

        using TicketDescriptorPtr = std::shared_ptr<TicketDescriptor>;

        /**
         * Wait for the currently executing ticket (and suspended tickets) to complete and prevent
         * any ticket from starting executing. Scheduling new tickets has no effect.
         */
        void stop_processing();

    private:
        struct State {
            std::mutex sync_mutex;
            std::thread::id processing_thread_id;
            std::condition_variable sync_condition;
            std::condition_variable idle_condition;

            // scheduled tickets
            std::list<TicketDescriptorPtr> processing_queue;

            // all created tickets that haven't completed running or haven't been discarded
            std::map<TicketHandle, TicketDescriptorPtr> registry;

            // currently executing (0..1) and suspended (0..n) tickets, see class description
            std::list<TicketDescriptorPtr> current_ticket_stack;
            bool stop_processing_requested = false;

            // whether the processing loop has been scheduled on the thread pool
            bool is_scheduled = false;

            std::shared_ptr<support::ThreadPool> thread_pool;
         };

        std::shared_ptr<State> _state;

        static void processing_loop(std::shared_ptr<State> state, std::function<bool()> recursion_break_condition);
        void schedule_processing_loop();

        /** @return nullptr when ticket not found */
        TicketDescriptorPtr get_descriptor(const OperationalQueue::TicketHandle&);

        bool is_in_current_ticket_stack(TicketDescriptorPtr);
    };
}  // namespace support
