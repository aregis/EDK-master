/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include "support/util/ReduceOperator.h"
#include "support/util/Operators.h"

namespace support {

    template <typename... Ts>
    using ReduceOrOperator = ReduceOperator<support::Or, Ts...>;

    template <typename... Ts>
    using ReduceAndOperator = ReduceOperator<support::And, Ts...>;

    template <typename... Ts>
    using ReduceAddOperator = ReduceOperator<support::Add, Ts...>;

    template <typename... Ts>
    using ReduceSubtractOperator = ReduceOperator<support::Subtract, Ts...>;

}  // namespace support
