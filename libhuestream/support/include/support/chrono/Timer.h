/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <memory>

using std::atomic;
using std::condition_variable;
using std::mutex;

namespace support {

    class Thread;
    typedef std::function<void ()> TimerEvent;
    
    class Timer {
    public:
        /**
         Construct timer with interval and event
         Timer event will be only be fired once !
         @param interval_ms The interval in milliseconds
         @param callback    The callback which will be called when the timer
                            interval elapsed
         */
        Timer(unsigned int interval_ms, TimerEvent callback);
        
        /**
         Construct (recurring) timer with interval and event
         @param interval_ms         The interval in milliseconds
         @param callback            The callback which will be called when the timer
                                    interval elapsed
         @param recurring           Whether the timer is recurring
         @param fire_event_on_start Whether the timer event should be fired on starting the timer
         */
        Timer(unsigned int interval_ms, TimerEvent callback, bool recurring, bool fire_event_on_start);
        
        /**
         Destructor, which will stop the timer and will block until the timer
         has been stopped
         */
        ~Timer();
        
        /**
         Get timer interval in ms
         @return The timer interval in ms
         */
        unsigned int get_interval_ms();
        
        /**
         Whether the timer is recurring
         @return true if the timer is recurring, false otherwise
         */
        bool is_recurring();
        
        /**
         Start the timer
         */
        void start();
        
        /**
         Stop the timer. This method will block until the timer has been 
         stopped
         */
        void stop();
        
    private:
        /** the timer will be handled in a seperate thread */
        std::unique_ptr<Thread>             _ticker_thread;
        /** ensure thread safe way of stopping the timer */
        mutex              _ticker_mutex;
        /** condition variable will be used to block until the interval elapsed */
        condition_variable _ticker_condition;
        /** whether the timer is running */
        atomic<bool>       _running;
        /** ensure timer can only be started once */
        mutex              _running_mutex;
        
        /** the timer interval in ms */
        unsigned int       _interval_ms;
        /** whether the timer is recurring */
        bool               _recurring;
        /** whether the timer event should be fired on starting the timer */
        bool               _fire_event_on_start;
        /** whether the timer should stop */
        atomic<bool>       _stop;
        /** the event to fire */
        TimerEvent         _event;
    };
    
}  // namespace support

