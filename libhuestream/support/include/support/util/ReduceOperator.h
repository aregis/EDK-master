/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include "support/util/TupleUtil.h"

#include <vector>
#include <tuple>
#include <algorithm>

#include "boost/optional.hpp"

namespace support {

    template <typename Operator, typename... Ts>
    class ReduceOperator {
    public:
        void operator()(Ts... args) {
            if (_data != boost::none) {
                tuple_for_each<Operator, sizeof...(args) - 1>{}(*_data, *_data, std::make_tuple(args...));
            } else {
                _data = std::make_tuple(args...);
            }
        }

        boost::optional<std::tuple<Ts...>> get() const {
            return _data;
        }

    private:
        boost::optional<std::tuple<Ts...>> _data;
    };

}  // namespace support
