/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <condition_variable>
#include <deque>
#include <mutex>

namespace support {

    /**
     * Multi-producer single-consumer queue
     * To be used as a communication pipe between two threads
     */
    template<typename T>
    class BlockingQueue {
    public:
        void push_back(T item) {
            std::unique_lock<std::mutex> lk(_mutex);

            _queue.push_back(item);
            _cond.notify_one();
        }

        T pop_front() {
            std::unique_lock<std::mutex> lk(_mutex);

            if (_queue.empty()) {
                return T();
            } else {
                T item = _queue.front();
                _queue.pop_front();
                return item;
            }
        }

        void wait_for_data() {
            std::unique_lock<std::mutex> lk(_mutex);

            _cond.wait(lk, [this] { return !_queue.empty(); });
        }

    private:
        std::deque<T>           _queue;
        std::mutex              _mutex;
        std::condition_variable _cond;
    };

}  // namespace support

