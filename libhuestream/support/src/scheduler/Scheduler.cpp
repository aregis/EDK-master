/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <algorithm>
#include <chrono>
#include <functional>
#include <map>
#include <memory>
#include <utility>
#include <vector>

#include "support/date/Date.h"
#include "support/logging/Log.h"
#include "support/scheduler/Scheduler.h"


using std::bind;
using std::map;
using std::sort;
using std::unique_lock;
using std::chrono::milliseconds;

namespace support {

    Scheduler::Scheduler(unsigned int timer_interval_ms) : _state(SCHEDULER_STATE_IDLE),
                                                           _timer(timer_interval_ms, bind(&Scheduler::thread_timer_task, this), true, true)
    { }
    
    Scheduler::~Scheduler() {
        stop();
    }

    void Scheduler::add_task(SchedulerTask task) {
        Date date;
        // Calculate the next occurence based on the current date + interval
        milliseconds next_occurence_ms = date.get_time_ms() + milliseconds(task.get_interval_ms());
        
        add_task(task, next_occurence_ms);
    }
    
    void Scheduler::add_task(SchedulerTask task, milliseconds next_occurence_ms) {
        unique_lock<mutex> scheduler_lock(_scheduler_mutex);
        
        // Create the schedule entry which contains the task and the time of the next occurence
        auto entry = support::make_unique<SchedulerEntry>(task, next_occurence_ms);
        
        // Remove the existing task with the same id
        remove_task_internal(task.get_id());
        
        // Schedule the task
        _scheduled_tasks.push_back(std::move(entry));
    }

    const SchedulerTask* Scheduler::get_task(int id) {
        unique_lock<mutex> scheduler_lock(_scheduler_mutex);

        SchedulerTask* task = nullptr;

        for (const auto& entry : _scheduled_tasks) {
            if (entry->task.get_id() == id) {
                task = &entry->task;
                
                break;
            }
        }

        return task;
    }

    size_t Scheduler::get_task_count() const {
        unique_lock<mutex> scheduler_lock(_scheduler_mutex);
        
        return _scheduled_tasks.size();
    }

    void Scheduler::remove_task(int id) {
        unique_lock<mutex> scheduler_lock(_scheduler_mutex);

        remove_task_internal(id);
    }
    
    void Scheduler::remove_all_tasks() {
        unique_lock<mutex> scheduler_lock(_scheduler_mutex);
    
        // Remove all the tasks
        _scheduled_tasks.clear();
    }

    void Scheduler::start() {
        unique_lock<mutex> scheduler_lock(_scheduler_mutex);
        
        if (_state == SCHEDULER_STATE_IDLE) {
            // Start the timer
            _timer.start();
            
            _state = SCHEDULER_STATE_RUNNING;
        }
    }
    
    void Scheduler::stop() {
        unique_lock<mutex> scheduler_lock(_scheduler_mutex);
            
        if (_state == SCHEDULER_STATE_RUNNING) {
            // Stop the scheduler
            _state = SCHEDULER_STATE_STOPPING;
        }

        scheduler_lock.unlock();

        // Stop the timer
        _timer.stop();
        
        scheduler_lock.lock();
        
        _state = SCHEDULER_STATE_IDLE;
    }

    bool Scheduler::is_running() const {
        return _state == SCHEDULER_STATE_RUNNING;
    }
    
    
    /* private */
    
    void Scheduler::remove_task_internal(int id) {
        remove_if(
                _scheduled_tasks,
                [id](const std::unique_ptr<SchedulerEntry>& item){ return item->task.get_id() == id; });
    }

    void Scheduler::thread_timer_task() {
        unique_lock<mutex> scheduler_lock(_scheduler_mutex);

        Date date;
        // Get current time in milliseconds
        milliseconds current_time_ms = date.get_time_ms();
        
        // Map with tasks to execute. Tasks will automatically be sorted on the occurrence time
        std::vector<std::unique_ptr<SchedulerEntry>> entries_to_execute;
        
        for (auto it = _scheduled_tasks.begin(); it != _scheduled_tasks.end();) {
            if (_state != SCHEDULER_STATE_STOPPING) {
                const auto& entry = *it;
                // Check whether the task should be executed by comparing the current time and the scheduled time
                if (entry->next_occurence_ms <= current_time_ms) {
                    // Check whether the task should be rescheduled
                    if (entry->task.is_recurring()) {
                        // Calculate the next occurence
                        entry->next_occurence_ms = current_time_ms + milliseconds(entry->task.get_interval_ms());
                        
                        // Ask task to the list to be executed
                        entries_to_execute.push_back(support::make_unique<SchedulerEntry>(entry->task, entry->next_occurence_ms));
                        
                        it++;
                    } else {
                        entries_to_execute.push_back(support::make_unique<SchedulerEntry>(entry->task, entry->next_occurence_ms));

                        // Remove the task
                        it = _scheduled_tasks.erase(it);
                    }
                } else {
                    it++;
                }
            } else {
                // Stop executing tasks
                break;
            }
        }
        
        scheduler_lock.unlock();
        
        if (_state != SCHEDULER_STATE_STOPPING) {
            /**
             Ordering example:
             
             before:
             [ 
                Task1 -> { next_occurence: 2ms },
                Task2 -> { next_occurence: 2ms },
                Task3 -> { next_occurence: 1ms },
                Task4 -> { next_occurence: 2ms },
             ]
             
             after:
             [ 
                Task3 -> { next_occurence: 1ms },
                Task1 -> { next_occurence: 2ms },
                Task2 -> { next_occurence: 2ms },
                Task4 -> { next_occurence: 2ms },
             ]
             */
        
            // Sort the scheduler tasks based on the next occurence
            sort(entries_to_execute.begin(),
                 entries_to_execute.end(),
                 [&] (const std::unique_ptr<SchedulerEntry>& entry1, const std::unique_ptr<SchedulerEntry>& entry2) -> bool {
                        return entry1->next_occurence_ms < entry2->next_occurence_ms;
                 });
            
            for (const auto& entry_to_execute : entries_to_execute) {
                entry_to_execute->task();
            }
        }
    }
}  // namespace support
