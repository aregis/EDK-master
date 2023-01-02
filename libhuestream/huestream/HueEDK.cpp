/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include "huestream/HueEDK.h"

#include "support/network/http/HttpRequest.h"
#include "support/threading/QueueExecutor.h"
#include "support/threading/QueueDispatcher.h"
#include "support/threading/ThreadPool.h"

using support::GlobalQueueExecutor;
using support::GlobalQueueDispatcher;
using support::GlobalThreadPool;

namespace {
    template<typename T>
    void reset_provider() {
        if (T::get() == nullptr) {
            T::set(default_object<typename T::type>::get());
        };
    }
}

namespace huestream {

    void HueEDK::init() {
        reset_provider<GlobalThreadPool>();
        reset_provider<GlobalQueueExecutor>();
        reset_provider<GlobalQueueDispatcher>();
    }

    void HueEDK::deinit() {
        support::HttpRequest::set_default_http_client(nullptr);

        GlobalQueueExecutor::get()->shutdown();
        GlobalQueueDispatcher::get()->shutdown();
        GlobalQueueDispatcher::get()->get_operational_queue()->get_thread_pool()->shutdown();
        GlobalThreadPool::get()->shutdown();
        
        GlobalQueueExecutor::set(nullptr);
        GlobalQueueDispatcher::set(nullptr);
        GlobalThreadPool::set(nullptr);
    }

}  // namespace huestrean
