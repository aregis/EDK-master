/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include "support/threading/SynchronousDispatcher.h"
#include "support/signals/detail/PeriodicSignal.h"
#include "support/signals/detail/PeriodicSignalScheduler.h"

namespace support {

    template<template<typename...> class Operation, typename... Ts>
    using PeriodicSynchronousSignal = detail::PeriodicSignal<SynchronousDispatcher, Operation, Ts...>;

}  //  namespace support
