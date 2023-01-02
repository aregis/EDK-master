/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <memory>
#include <utility>

#include "gmock/gmock.h"

#include "support/threading/ThreadPoolExecutor.h"

namespace support {

    class MockThreadPoolExecutor : public ThreadPoolExecutor {
    public:
        explicit MockThreadPoolExecutor(ThreadPoolExecutor::ShutdownPolicy shutdown_policy = ThreadPoolExecutor::ShutdownPolicy::CANCEL_ALL)
                : ThreadPoolExecutor{shutdown_policy} {}
        explicit MockThreadPoolExecutor(std::shared_ptr<ThreadPool> thread_pool, ThreadPoolExecutor::ShutdownPolicy shutdown_policy = ThreadPoolExecutor::ShutdownPolicy::CANCEL_ALL)
                : ThreadPoolExecutor{std::move(thread_pool), shutdown_policy} {};

        MOCK_METHOD0(wait_all, void());
        MOCK_METHOD0(cancel_all, void());
        MOCK_METHOD0(shutdown, void());
        MOCK_METHOD2(execute, support::Operation(std::function<void()>, OperationType));
    };

}  // namespace support
