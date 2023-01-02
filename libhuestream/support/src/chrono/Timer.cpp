/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/
 
#include <chrono>

#include "support/chrono/Timer.h"
#include "support/threading/Thread.h"

using std::unique_lock;
using std::chrono::milliseconds;

namespace support {
            
    Timer::Timer(unsigned int interval_ms, TimerEvent event) : _ticker_thread(new support::Thread),
                                                               _running(false),
                                                               _interval_ms(interval_ms),
                                                               _recurring(false),
                                                               _fire_event_on_start(false),
                                                               _stop(false),
                                                               _event(event) { }
    
    Timer::Timer(unsigned int interval_ms, TimerEvent event, bool recurring, bool fire_event_on_start) :
                                                                               _ticker_thread(new support::Thread),
                                                                               _running(false),
                                                                               _interval_ms(interval_ms),
                                                                               _recurring(recurring),
                                                                               _fire_event_on_start(fire_event_on_start),
                                                                               _stop(false),
                                                                               _event(event) { }

    Timer::~Timer() {
        // Stop the timer
        stop();
    }
    
    unsigned int Timer::get_interval_ms() {
        return this->_interval_ms;
    }
    
    bool Timer::is_recurring() {
        return this->_recurring;
    }

    void Timer::start() {
        unique_lock<mutex> running_lock(_running_mutex);
    
        if (!_running) {
            _ticker_thread = std::make_unique<support::Thread>([this] () {
                if (!_stop
                    && _fire_event_on_start) {
                    _event();
                }
            
                while (true
                    && !_stop) {
                    {
                        unique_lock<mutex> ticker_lock(_ticker_mutex);
                
                        // Block until the timeout or the timer has been stopped
                        _ticker_condition.wait_for(ticker_lock, milliseconds(_interval_ms));
                    }
                    
                    // Check if timer has been stopped in meantime. If so, don't fire the event
                    if (!_stop) {
                        // Execute the timer event
                        _event();
                    }
                    
                    if (!_recurring) {
                        // Stop the timer thread, since it should only be executed once
                        break;
                    }
                }
                
                unique_lock<mutex> ticker_lock(_running_mutex);
                        
                _running = false;
                _stop    = false;
            });
            
            _running = true;
        }
    }
    
    void Timer::stop() {
        {  // Lock
            unique_lock<mutex> ticker_lock(_ticker_mutex);
            
            _stop = true;
        
            _ticker_condition.notify_one();
        }

        // Join the ticker thread until it's closed
        _ticker_thread->join();
    }
    
}  // namespace support
