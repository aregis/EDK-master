/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <huestream/connect/MessageDispatcher.h>

#include <iostream>
#include <memory>

namespace huestream {

MessageDispatcher::MessageDispatcher() :
    _isRunning(false) {
}

void MessageDispatcher::Queue(DispatchAction action) {
    std::unique_lock<std::mutex> lock(_mutex);
    _queue.insert(_queue.begin(), action);
    lock.unlock();
    _condition.notify_all();
}

void MessageDispatcher::Execute(bool useThisTread) {
    {
        std::unique_lock<std::mutex> lock(_mutex);
        _isRunning = false;
    }

    if (useThisTread) {
        Loop();
    } else {
        support::ThreadTask task = [this]() -> void { Loop(); };
        _thread = std::unique_ptr<support::Thread>(new support::Thread(task));
    }
    WaitUntilStarted();
}
void MessageDispatcher::WaitUntilStarted()  {
    while (true) {
        std::unique_lock<std::mutex> lock(_mutex);
        _condition.wait(lock, [this]() -> bool {
            return _isRunning;
        });
        if (_isRunning)
            break;
    }
}

void MessageDispatcher::Loop() {
    {
        std::unique_lock<std::mutex> lock(_mutex);
        _isRunning = true;
        _condition.notify_all();
    }

    while (true) {
        std::unique_lock<std::mutex> lock(_mutex);
        _condition.wait(lock, [this]() -> bool {
            return _queue.size() != 0 || !_isRunning;
        });
        if (!_isRunning)
            break;
        auto action = _queue.back();
        _queue.pop_back();
        lock.unlock();
        action();
    }
}

void MessageDispatcher::Stop() {
    {
        std::unique_lock<std::mutex> lock(_mutex);
        _isRunning = false;
        _condition.notify_all();
    }

    if (_thread != nullptr) {
        _thread->join();
        _thread.reset();
    }
}

}  // namespace huestream
