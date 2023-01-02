/*******************************************************************************
Copyright (C) 2019 Signify Holding
All Rights Reserved.
********************************************************************************/

#include <utility>

#include "support/threading/OperationalQueue.h"
#include "support/threading/QueueDispatcher.h"
#include "support/threading/QueueExecutor.h"

namespace support {

    QueueDispatcher::QueueDispatcher(bool wait_all)
            : QueueDispatcher(GlobalQueueDispatcher::get()->get_operational_queue(), wait_all) {
    }
    
    QueueDispatcher::QueueDispatcher(std::shared_ptr<OperationalQueue> queue, bool wait_all)
            : _executor(std::unique_ptr<QueueExecutor>(new QueueExecutor(queue)))
            , _wait_all(wait_all) {
    }

    QueueDispatcher::~QueueDispatcher() {
    }

    support::Operation QueueDispatcher::post(std::function<void()> invocation) {
        return _executor->execute(std::move(invocation), _wait_all ?
            QueueExecutor::OperationType::NON_CANCELABLE : QueueExecutor::OperationType::CANCELABLE);
    }

    void QueueDispatcher::shutdown() {
        _executor->shutdown();
    }

    std::shared_ptr<OperationalQueue> QueueDispatcher::get_operational_queue() const {
        return _executor->get_operational_queue();
    }

}  // namespace support
