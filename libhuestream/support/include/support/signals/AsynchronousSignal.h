/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include "support/signals/detail/Signal.h"
#include "support/threading/QueueDispatcher.h"

namespace support {

    template<typename... Ts>
    using AsynchronousSignal = detail::Signal<QueueDispatcher, detail::InstantaneousScheduler<Ts...>, Ts...>;

}  //  namespace support
