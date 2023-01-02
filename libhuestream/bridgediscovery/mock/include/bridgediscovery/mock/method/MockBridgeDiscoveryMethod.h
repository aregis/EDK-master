/*******************************************************************************
 Copyright (C) 2018 Philips Lighting Holding B.V.
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <gmock/gmock.h>

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <vector>

#include "support/threading/Thread.h"
#include "support/threading/QueueDispatcher.h"

#include "bridgediscovery/BridgeDiscoveryClassType.h"
#include "bridgediscovery/BridgeDiscoveryResult.h"
#include "bridgediscovery/IBridgeDiscoveryCallback.h"
#include "bridgediscovery/BridgeDiscoveryReturnCode.h"

#include "method/IBridgeDiscoveryMethod.h"

using std::atomic;
using std::condition_variable;
using std::mutex;
using std::unique_ptr;
using huesdk::BridgeDiscoveryResult;
using huesdk::IBridgeDiscoveryCallback;
using huesdk::IBridgeDiscoveryMethod;
using huesdk::BridgeDiscoveryReturnCode;
using huesdk::BridgeDiscoveryClassType;
using support::Thread;
using support::QueueDispatcher;
using testing::Invoke;
using testing::_;

namespace support_unittests {

    class MockBridgeDiscoveryMethod : public IBridgeDiscoveryMethod {
    public:
        MockBridgeDiscoveryMethod() : _searching(false) {
            // Delegate mocked calls to default implementation
            ON_CALL(*this, search(_))
                .WillByDefault(Invoke(this, &MockBridgeDiscoveryMethod::default_search));
            ON_CALL(*this, is_searching())
                .WillByDefault(Invoke(this, &MockBridgeDiscoveryMethod::default_is_searching));
            ON_CALL(*this, stop())
                .WillByDefault(Invoke(this, &MockBridgeDiscoveryMethod::default_stop));
            ON_CALL(*this, get_type())
                .WillByDefault(Invoke(this, &MockBridgeDiscoveryMethod::default_get_type));
            ON_CALL(*this, destroy())
                .WillByDefault(Invoke(this, &MockBridgeDiscoveryMethod::default_destroy));
        }
        
        virtual ~MockBridgeDiscoveryMethod() {
        }

        /**
        
         */
        MOCK_METHOD1(search, void(std::shared_ptr<IBridgeDiscoveryCallback> callback));
 
        /** 
        
         */
        MOCK_METHOD0(is_searching, bool());
        
        /** 
        
         */
        MOCK_METHOD0(stop, void());
        
        /** 
        
         */
        MOCK_CONST_METHOD0(get_type, BridgeDiscoveryClassType());
        
        /**
        
         */
        MOCK_CONST_METHOD0(destroy, void());
        
        /* fake data */

        /** 
        
         */
        MOCK_METHOD0(fake_delay, unsigned int());
        
        /** 
        
         */
        MOCK_METHOD0(fake_results, std::vector<std::shared_ptr<BridgeDiscoveryResult>>());
        
    protected:
        /** */
        std::atomic<bool>  _searching;
        /** */
        QueueDispatcher    _dispatcher;
        /** */
        mutex              _searching_mutex;
        /** */
        unique_ptr<Thread> _searching_thread;
        /** */
        mutex              _stop_mutex;

        /**
         
         */
        void default_search(std::shared_ptr<IBridgeDiscoveryCallback> callback);
        
        /**
        
         */
        bool default_is_searching();
        
        /**
        
         */
        void default_stop();
        
        /**
        
         */
        BridgeDiscoveryClassType default_get_type() const;
 
        /**

         */
        void default_destroy() const;

        /**
        
         */
        void thread_search(std::shared_ptr<IBridgeDiscoveryCallback> callback);
    };
    
}  // namespace support_unittests

