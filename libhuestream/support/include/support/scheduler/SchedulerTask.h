/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <functional>

using std::function;

namespace support {

    // Definition of the task method lambda, which will represents the task to be executed
    typedef function<void ()> SchedulerTaskMethod;

    class SchedulerTask {
    public:
        /**
         Construct empty scheduler task
         */
        SchedulerTask();
    
        /**
         Construct the scheduler task with an id, interval and method to be executed
         @param id          The identifier of the task
         @param interval_ms The interval in milliseconds
         @param method      The method to be executed
         */
        SchedulerTask(int id, unsigned int interval_ms, SchedulerTaskMethod method);
    
        /**
         Get the identifier of the task
         @return The identifier of the task
         */
        int get_id() const;
        
        /**
         Set the id of the task
         @param id The identifier of the task
         */
        void set_id(int id);
        
        /**
         Get interval
         @return The interval in milliseconds
         */
        unsigned int get_interval_ms() const;
        
        /**
         Set the interval
         @param interval_ms The interval is milliseconds
         */
        void set_interval_ms(unsigned int interval_ms);
        
        /**
         Whether the task is running
         @return Whether the task is recurring
         */
        bool is_recurring() const;
        
        /**
         Set whether the task is recurring
         @param recurring Whether the task is recurring
         */
        void set_recurring(bool recurring);
        
        /**
         Get the method to be executed
         @return The method to be executed
         */
        SchedulerTaskMethod get_method() const;
        
        /**
         Set the method to be executed
         @param method The method to be executed
         */
        void set_method(SchedulerTaskMethod method);
        
        /**
         Functor which makes it possible to call this task as an actual method.
         Internally the set method will be called
         */
        void operator()() const;
        
    private:
        /** the unique id of the task */
        int                 _id;
        /** the interval in milliseconds */
        unsigned int        _interval_ms;
        /** whether the task is recurring  */
        bool                _recurring;
        /** the method to be executed */
        SchedulerTaskMethod _method;
    };

}  // namespace support

