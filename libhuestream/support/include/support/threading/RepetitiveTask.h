/*******************************************************************************
Copyright (C) 2019 Signify Holding
All Rights Reserved.
********************************************************************************/

#pragma once

#include <functional>
#include <utility>
#include <memory>

#include "support/threading/Executor.h"

namespace support {
    class RepetitiveTask {
    public:
        RepetitiveTask(support::Executor* executor, std::function<void()> invocable)
                : _executor(executor)
                , _invocable(std::move(invocable)) {}

        operator std::function<void()>() const {
            return std::bind(execute, _executor, std::move(_invocable));
        }

    private:
        static void execute(support::Executor* executor, std::function<void()> invocable) {
            invocable();
            executor->execute(RepetitiveTask(executor, std::move(invocable)));
        }

        support::Executor* _executor;
        std::function<void()> _invocable;
    };
}  // namespace support