/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include "support/scheduler/SchedulerTask.h"

namespace support {
    SchedulerTask::SchedulerTask() : _id(0),
                                     _interval_ms(0),
                                     _recurring(false),
                                     _method(nullptr) {}

    SchedulerTask::SchedulerTask(int id, unsigned int interval_ms, SchedulerTaskMethod method) : _id(id),
                                                                                                 _interval_ms(interval_ms),
                                                                                                 _recurring(true),
                                                                                                 _method(method) { }
    
    int SchedulerTask::get_id() const {
        return _id;
    }

    void SchedulerTask::set_id(int id) {
        _id = id;
    }

    unsigned int SchedulerTask::get_interval_ms() const {
        return _interval_ms;
    }

    void SchedulerTask::set_interval_ms(unsigned int interval_ms) {
        _interval_ms = interval_ms;
    }

    bool SchedulerTask::is_recurring() const {
        return _recurring;
    }

    void SchedulerTask::set_recurring(bool recurring) {
        _recurring = recurring;
    }

    SchedulerTaskMethod SchedulerTask::get_method() const {
        return _method;
    }

    void SchedulerTask::set_method(SchedulerTaskMethod method) {
        _method = method;
    }
    
    void SchedulerTask::operator()() const {
        // Execute the method
        _method();
    }

}  // namespace support
