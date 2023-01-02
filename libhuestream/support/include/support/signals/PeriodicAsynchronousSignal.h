/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once


#include "support/threading/QueueDispatcher.h"
#include "support/signals/detail/PeriodicSignal.h"
#include "support/signals/detail/PeriodicSignalScheduler.h"

namespace support {

    template<template<typename...> class Operation, typename... Ts>
    using PeriodicAsynchronousSignal = detail::PeriodicSignal<QueueDispatcher, Operation, Ts...>;

}  //  namespace support
