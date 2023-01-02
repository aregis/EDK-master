/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include "support/signals/detail/Signal.h"
#include "support/threading/SynchronousDispatcher.h"


namespace support {

    template<typename... Ts>
    using SynchronousSignal = detail::Signal<SynchronousDispatcher, detail::InstantaneousScheduler<Ts...>, Ts...>;

}  //  namespace support
