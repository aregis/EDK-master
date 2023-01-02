/*******************************************************************************
Copyright (C) 2019 Signify Holding
All Rights Reserved.
********************************************************************************/
#include <mutex>

#include "support/threading/ReadWriteLock.h"

using std::mutex;
using std::unique_lock;
using std::lock_guard;

namespace support {

    ReadWriteLock::ReadWriteLock() :
    _reader_count(0),
    _has_write_lock(false),
    _stop(false),
    _busy(false) {
    }

    ReadWriteLock::~ReadWriteLock() {
        unique_lock<mutex> lock(_mutex);
        _stop = true;
        _unlocked_condition.notify_all();
        
        if (_busy) {
            _busy_condition.wait(lock, [&] () -> bool {
                return !_busy;
            });
        }
    }

    void ReadWriteLock::read_lock() {
        unique_lock<mutex> lock(_mutex);
        _busy = true;
        if (_has_write_lock) {
            _unlocked_condition.wait(lock, [&] () -> bool {
                return (!_has_write_lock || _stop);
            });
        }
        _reader_count++;
        _busy = false;
        _busy_condition.notify_all();
    }
    
    void ReadWriteLock::read_unlock() {
        lock_guard<mutex> lock(_mutex);
        _reader_count--;
        if (_reader_count == 0)
            _unlocked_condition.notify_all();
    }
    
    void ReadWriteLock::write_lock() {
        unique_lock<mutex> lock(_mutex);
        _busy = true;
        if (_has_write_lock || (_reader_count > 0)) {
            _unlocked_condition.wait(lock, [&] () -> bool {
                return ((!_has_write_lock && (_reader_count == 0)) || _stop);
            });
        }
        // only has write lock if we didn't stop
        _has_write_lock = !_stop;
        _busy = false;
        _busy_condition.notify_all();
    }
    
    void ReadWriteLock::write_unlock() {
        lock_guard<mutex> lock(_mutex);
        _has_write_lock = false;
        _unlocked_condition.notify_all();
    }
    
    UniqueReadLock::UniqueReadLock(ReadWriteLock* lock) {
        _lock = lock;
        lock->read_lock();
    }
    
    UniqueReadLock::~UniqueReadLock() {
        _lock->read_unlock();
    }
   
    UniqueWriteLock::UniqueWriteLock(ReadWriteLock* lock) {
        _lock = lock;
        _lock->write_lock();
    }
    
    UniqueWriteLock::~UniqueWriteLock() {
        _lock->write_unlock();
    }

}  // namespace support
