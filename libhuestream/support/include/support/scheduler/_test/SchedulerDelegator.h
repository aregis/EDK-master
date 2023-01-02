/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <memory>
#include <string>

#include "support/scheduler/Scheduler.h"

using std::shared_ptr;
using std::string;

namespace support {

    class SchedulerDelegateProvider {
    public:
        /**
         Get scheduler delegate
         @return The scheduler delegate
         */
        virtual shared_ptr<Scheduler> get_delegate(unsigned int timer_interval_ms) = 0;
    };
    
    // Default
    class SchedulerDelegateProviderImpl : public SchedulerDelegateProvider {
    public:
        /**
         @see SchedulerDelegateProvider
         */
        shared_ptr<Scheduler> get_delegate(unsigned int timer_interval_ms) {
            // Create instance of the real scheduler implementation
            return shared_ptr<Scheduler>(new Scheduler(timer_interval_ms));
        }
    };

    class SchedulerDelegator : public Scheduler {
    public:
        /**
         @see lib/scheduler/Scheduler.h
         */
        explicit SchedulerDelegator(unsigned int timer_interval_ms);
    
        /**
         @see lib/scheduler/Scheduler.h
         */
        virtual void add_task(SchedulerTask task);
        
        /**
         @see lib/scheduler/Scheduler.h
         */
        virtual void add_task(SchedulerTask task, milliseconds next_occurence_ms);

        /**
         @see lib/scheduler/Scheduler.h
         */
        virtual const SchedulerTask* get_task(int id);
        
        /**
         @see lib/scheduler/Scheduler.h
         */
        virtual size_t get_task_count() const;
        
        /**
         @see lib/scheduler/Scheduler.h
         */
        virtual void remove_task(int id);
        
        /**
         @see lib/scheduler/Scheduler.h
         */
        virtual void remove_all_tasks();
    
        /**
         @see lib/scheduler/Scheduler.h
         */
        virtual void start();
        
        /**
         @see lib/scheduler/Scheduler.h
         */
        virtual void stop();
        
        /**
         @see lib/scheduler/Scheduler.h
         */
        virtual bool is_running() const;
        
        /* delegate provider */
        
        /**
         Set the delegate
         @note   Initially SchedulerDelegateProviderImpl is set as delegate
         @return The delegate, nullptr if no delegate has been set
         */
        static void set_delegate_provider(shared_ptr<SchedulerDelegateProvider> delegate_provider);
        
    private:
        /** the scheduler delegate */
        shared_ptr<Scheduler> _delegate;
    };

}  // namespace support
