/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <memory>
#include <utility>

#include "support/threading/OperationalQueue.h"
#include "support/threading/QueueExecutor.h"

namespace support {

    template <typename T>
    void delete_later(T* object) {
        static QueueExecutor delete_executor(std::make_shared<OperationalQueue>());
        delete_executor.execute([object] {
                delete object;
        });
    }

    template <typename T, typename U>
    void delete_later(std::unique_ptr<T, U>&& object) {
        delete_later(new std::shared_ptr<T>(std::move(object)));
    }

    template <typename T>
    void delete_later(std::shared_ptr<T>&& object) {
        delete_later(new std::shared_ptr<T>(std::move(object)));
    }

}  // namespace support

