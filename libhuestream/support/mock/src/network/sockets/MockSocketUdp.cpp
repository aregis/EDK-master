/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <chrono>
#include <functional>
#include <string>

#include "support/mock/network/sockets/MockSocketUdp.h"

using std::bind;
using std::unique_lock;
using std::string;
using std::chrono::milliseconds;
using std::this_thread::sleep_for;
using support::SocketStatusCode;
using support::SOCKET_STATUS_FAILED;
using support::SOCKET_STATUS_OK;
using support::SocketAddress;
using support::SocketCallback;
using support::SocketError;
using support::ThreadTask;

namespace support_unittests {

    SocketStatusCode MockSocketUdp::default_bind() {
        _bounded = true;
        
        return SOCKET_STATUS_OK;
    }
    
    SocketStatusCode MockSocketUdp::default_connect(const SocketAddress& /*address_remote*/, SocketCallback /*callback*/) {
        return SOCKET_STATUS_FAILED;
    }
    
    void MockSocketUdp::default_close() {
        {  // Lock
            unique_lock<mutex> socket_lock(_socket_mutex);
        
            if (_opened) {
                _opened    = false;
                _bounded   = false;
            }
        }
        
        if (_sending_thread != nullptr) {
            // Wait until the sending thread is finished
            _sending_thread->join();
        }
        if (_receiving_thread != nullptr) {
            // Wait until the receiving thread is finished
            _receiving_thread->join();
        }
    }

    bool MockSocketUdp::default_is_opened() {
        return _opened;
    }

    bool MockSocketUdp::default_is_bounded() {
        return _bounded;
    }
    
    int MockSocketUdp::default_async_send(const string& /*data*/, SocketCallback /*callback*/) {
        return SOCKET_STATUS_FAILED;
    }
    
    int MockSocketUdp::default_async_send2(const string& data, const SocketAddress& address_remote, SocketCallback callback) {
        unique_lock<mutex> socket_lock(_socket_mutex);
        
        // Check whether the socket is not opened or already sending
        if (!_opened
            || _sending) {
            return SOCKET_STATUS_FAILED;
        }
        
        int status = SOCKET_STATUS_OK;
        
        if (_sending_thread != nullptr) {
            // Wait until the sending thread is finished
            _sending_thread->join();
            
            _sending_thread = nullptr;
        }
        
        // Initialize sending task
        ThreadTask task = ::bind(&MockSocketUdp::thread_send, this, data, address_remote, callback);
        // Start thread with the task
        _sending_thread = unique_ptr<Thread>(new Thread(task));

        _sending = true;

        return status;
    }
    
    int MockSocketUdp::default_async_receive(string& data, size_t buffer_size, SocketCallback callback) {  // NOLINT
        unique_lock<mutex> socket_lock(_socket_mutex);
        
        // Check whether the socket is not opened or already receiving
        if (!_opened
            || _receiving) {
            return SOCKET_STATUS_FAILED;
        }
        
        int status = SOCKET_STATUS_OK;
        
        if (_receiving_thread != nullptr) {
            // Wait until the receiving thread is finished
            _receiving_thread->join();
            
            _receiving_thread = nullptr;
        }
        
        // Initialize receiving task
        ThreadTask task = ::bind(&MockSocketUdp::thread_receive, this, &data, buffer_size, callback);
        // Start thread with the task
        _sending_thread = unique_ptr<Thread>(new Thread(task));
    
        _receiving = true;
        
        return status;
    }
    
    int MockSocketUdp::default_async_receive2(string& /*data*/, size_t /*buffer_size*/, const SocketAddress& /*address_remote*/, SocketCallback /*callback*/) {  // NOLINT
        return SOCKET_STATUS_FAILED;
    }
    
    int MockSocketUdp::default_get_reuse_address() const {
        // Fake
        return 1;
    }

    int MockSocketUdp::default_set_reuse_address(int /*reuse_address*/) {
        return SOCKET_STATUS_OK;
    }

    int MockSocketUdp::default_get_broadcast() const {
        // Fake
        return 1;
    }

    int MockSocketUdp::default_set_broadcast(int /*broadcast*/) {
        return SOCKET_STATUS_OK;
    }

    int MockSocketUdp::default_join_group(const SocketAddress& /*address_multicast*/) {
        return SOCKET_STATUS_OK;
    }

    int MockSocketUdp::default_leave_group(const SocketAddress& /*address_multicast*/) {
        return SOCKET_STATUS_OK;
    }
    
    void MockSocketUdp::thread_send(string data, SocketAddress /*address_remote*/, SocketCallback callback) {
        // Get fake delay
        unsigned int delay = fake_sending_delay();
        // Get fake error
        SocketError error  = fake_sending_error();
        
        // Wait for a while before calling the callback
        sleep_for(milliseconds(delay));

        {  // Lock
            unique_lock<mutex> socket_lock(_socket_mutex);
        
            // Done sending
            _sending = false;
        }
        
        
        /* callback */
        
        callback((const class SocketError&)error, (unsigned int)data.length());
    }
    
    void MockSocketUdp::thread_receive(string* data, size_t /*buffer_size*/, SocketCallback callback) {
        // Get fake delay
        unsigned int delay = fake_receiving_delay();
        // Get fake error
        SocketError error  = fake_receiving_error();
        // Get fake data
        *data              = fake_receiving_data();
        
        // Wait for a while before calling the callback
        sleep_for(milliseconds(delay));
        
        {  // Lock
            unique_lock<mutex> socket_lock(_socket_mutex);
        
            // Done receiving
            _receiving = false;
        }
        
        
        /* callback */
        
        callback((const class SocketError&)error, (unsigned int)data->size());
    }

}  // namespace support_unittests
