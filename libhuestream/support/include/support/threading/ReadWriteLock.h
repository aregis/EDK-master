/*******************************************************************************
Copyright (C) 2019 Signify Holding
All Rights Reserved.
********************************************************************************/
#pragma once

#include <mutex>
#include <condition_variable>

namespace support {
    
    class ReadWriteLock{
    public:
        /**
        constructor
        */
        ReadWriteLock();

        /**
        destructor
        */
        ~ReadWriteLock();
        
        /**
         obtain a read lock
         will block while write lock is active
         */
        void read_lock();

        /**
         release a read lock
         */
        void read_unlock();
        
        /**
         obtain a write lock
         will block while a read lock or write lock is active
         */
        void write_lock();
        
        /**
         release a write lock
         */
        void write_unlock();

    private:
        int  _reader_count;
        bool _has_write_lock;
        bool _stop;
        bool _busy;
        std::mutex _mutex;
        std::condition_variable _unlocked_condition;
        std::condition_variable _busy_condition;
    };
    
    class UniqueReadLock {
    public:
        explicit UniqueReadLock(ReadWriteLock* lock);
        ~UniqueReadLock();
    private:
        ReadWriteLock* _lock;
    };
    
    class UniqueWriteLock {
    public:
        explicit UniqueWriteLock(ReadWriteLock* lock);
        ~UniqueWriteLock();
    private:
        ReadWriteLock* _lock;
    };
    
}  // namespace support

