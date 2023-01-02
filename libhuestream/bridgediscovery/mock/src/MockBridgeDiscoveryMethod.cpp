/*******************************************************************************
 Copyright (C) 2018 Philips Lighting Holding B.V.
 All Rights Reserved.
 ********************************************************************************/

#include <utility>
#include <chrono>
#include <type_traits>
#include <vector>

#include "support/threading/Thread.h"

#include "bridgediscovery/BridgeDiscovery.h"
#include "bridgediscovery/BridgeDiscoveryReturnCode.h"
#include "bridgediscovery/IBridgeDiscoveryCallback.h"

#include "bridgediscovery/mock/method/MockBridgeDiscoveryMethod.h"

using std::bind;
using std::vector;
using std::this_thread::sleep_for;
using std::unique_lock;
using std::shared_ptr;
using std::chrono::milliseconds;
using support::Thread;
using support::ThreadTask;

namespace support_unittests {

    /* default implementation */

    void MockBridgeDiscoveryMethod::default_search(shared_ptr<IBridgeDiscoveryCallback> callback) {
        unique_lock<mutex> searching_lock(_searching_mutex);
        
        if (!_searching) {
            unique_lock<mutex> stop_lock(_stop_mutex);
        
            // Initialize search task
            ThreadTask task = bind(&MockBridgeDiscoveryMethod::thread_search, this, callback);
            // Start thread with the task
            _searching_thread = unique_ptr<Thread>(new Thread(task));
        } else {
            // Search already in progress
            vector<std::shared_ptr<BridgeDiscoveryResult>> results;
            (*callback)(results, huesdk::BRIDGE_DISCOVERY_RETURN_CODE_BUSY);
        }
    }

    bool MockBridgeDiscoveryMethod::default_is_searching() {
        return _searching;
    }
    
    void MockBridgeDiscoveryMethod::default_stop() {
        unique_lock<mutex> stop_lock(_stop_mutex);
        
        if (_searching_thread != nullptr) {
            // Wait until the searching thread is finished
            _searching_thread->join();
            
            _searching_thread = nullptr;
        }
    }

    BridgeDiscoveryClassType MockBridgeDiscoveryMethod::default_get_type() const {
        // Fake
        return huesdk::BRIDGE_DISCOVERY_CLASS_TYPE_UNDEFINED;
    }
    
    void MockBridgeDiscoveryMethod::default_destroy() const {
        delete(this);
    }

    void MockBridgeDiscoveryMethod::thread_search(shared_ptr<IBridgeDiscoveryCallback> callback) {
        // Get fake delay
        unsigned int delay                           = fake_delay();
        // Get fake results
        const vector<std::shared_ptr<BridgeDiscoveryResult>> &results = fake_results();
        BridgeDiscoveryReturnCode return_code = huesdk::BRIDGE_DISCOVERY_RETURN_CODE_SUCCESS;
        
        // Wait for a while before calling the callback
        sleep_for(milliseconds(delay));

        {  // Lock
            unique_lock<mutex> searching_lock(_searching_mutex);

            // Done searching
            _searching = false;
        }

        vector<std::shared_ptr<BridgeDiscoveryResult>> results_vector;
        for (auto item : results)
            results_vector.push_back(item);

        /* callback */
        _dispatcher.post([callback, results_vector, return_code]() {
            // Call callback from another thread
            (*callback)(results_vector, return_code);
        });
    }
    
}  // namespace support_unittests
