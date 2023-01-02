/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#if defined(__APPLE__)
#  include <pthread.h>
#  include <errno.h>
#elif defined(__linux__)
#  include <pthread.h>
#  include <sys/prctl.h>
#endif

#include <system_error>
#include <vector>

#include "support/threading/Thread.h"

using std::thread;
using std::system_error;

namespace support {

    Thread::Thread(function<void()> task)
        : Thread("", std::move(task)) {
    }

    Thread::~Thread() {
        join();
    }

    bool Thread::joinable() const {
        return _thread.joinable();
    }

    void Thread::join() {
        if (_thread.joinable()) {
            _thread.join();
        }
    }

    void Thread::detach() {
        if (_thread.joinable()) {
            _thread.detach();
        }
    }

    Thread::Thread(std::string name, ThreadTask task) {
        if (name.empty()) {
            _thread = thread(std::move(task));
        } else {
            _thread = thread([name, task] {
                Self::set_name(name);
                task();
            });
        }
    }

    std::thread::id Thread::get_id() const {
        return _thread.get_id();
    }

    void Thread::Self::set_name(const std::string& name) {
    #if defined(__APPLE__)
        pthread_setname_np(name.c_str());
    #elif defined(__linux__)
        pthread_setname_np(pthread_self(), name.c_str());
    #else
        // no implementation
        (void)name;
    #endif
    }

    std::string Thread::Self::name() {
        std::string result;
    #if defined(__APPLE__)
        const auto max_possible_symbols = 64;
        std::vector<char> buffer(max_possible_symbols, '\0');
        const int err = pthread_getname_np(pthread_self(), buffer.data(), buffer.size());
        if (err == 0) {
            result = buffer.data();
        }
    #elif defined(__linux__)
        const auto max_possible_symbols = 16;
        std::vector<char> buffer(max_possible_symbols, '\0');
        const int err = prctl(PR_GET_NAME, buffer.data());
        if (err == 0) {
            result = buffer.data();
        }
    #else
        // no implementation
    #endif
        return result;
    }
}  // namespace support
