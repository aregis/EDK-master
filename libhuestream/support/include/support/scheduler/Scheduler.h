/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/
#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <map>
#include <memory>
#include <vector>

#include "support/chrono/Timer.h"
#include "support/scheduler/SchedulerTask.h"

using std::atomic;
using std::condition_variable;
using std::mutex;
using std::shared_ptr;
using std::chrono::milliseconds;

namespace support {

    struct SchedulerEntry {
        SchedulerEntry(const SchedulerTask& task_, const milliseconds& next_occurence_ms_)
                : task(task_), next_occurence_ms(next_occurence_ms_) {}

        SchedulerTask task;
        milliseconds  next_occurence_ms;
    };

    enum SchedulerState {
        SCHEDULER_STATE_IDLE,
        SCHEDULER_STATE_RUNNING,
        SCHEDULER_STATE_STOPPING,
    };

    class Scheduler {
    public:
        /**
         Construct a scheduler with a timer interval.
         The timer interval respresents the ticking interval of the internal timer
         used by the scheduler
         @param timer_interval_ms The interval of the timer in milliseconds
         */
        explicit Scheduler(unsigned int timer_interval_ms);
        
        /**
         Destructor
         */
        virtual ~Scheduler();
    
        /**
         Add task to the scheduler
         @note if a task with the same id already exists, the task will be rescheduled
         @param task        The task to add
         */
        virtual void add_task(SchedulerTask task);
        
        /**
         Schedule task on specific time interval
         @note if a task with the same id already exists, the task will be rescheduled
         @param task              The task to add
         @param next_occurence_ms When the task should be executed
         */
        virtual void add_task(SchedulerTask task, milliseconds next_occurence_ms);

        /**
         Get task by identifier
         @param id The identifier of the task
         @return The scheduler task
         */
        virtual const SchedulerTask* get_task(int id);
        
        /**
         Get the number of tasks 
         @return The number of tasks
         */
        virtual size_t get_task_count() const;
        
        /**
         Remove task by id
         @parma id The identifier of the task
         */
        virtual void remove_task(int id);
        
        /**
         Remove all tasks from the scheduler
         */
        virtual void remove_all_tasks();
    
        /**
         Start the scheduler
         */
        virtual void start();
        
        /**
         Stop the scheduler. This method will block until the current executed
         task has been completed.
         @note The scheduler will be reset
         */
        virtual void stop();
        
        /**
         Whether the scheduler is running
         @return true when the scheduler is running, false otherwise
         */
        virtual bool is_running() const;
        
    private:
        /** map with scheduled tasks and the absolute time to be executed */
        std::vector<std::unique_ptr<SchedulerEntry>> _scheduled_tasks;
        /** state of the scheduler (e.g. running, idle, ...)*/
        atomic<SchedulerState>      _state;
        /** ensure thread safety when managing the map */
        mutable mutex               _scheduler_mutex;
        /** recurring timer which handles executing the tasks */
        Timer                       _timer;
        
        /**
         Remove task by id
         @parma id The identifier of the task
         */
        void remove_task_internal(int id);
        
        /**
         Timer task, which will scan for tasks to execute
         */
        void thread_timer_task();
    };

}  // namespace support

