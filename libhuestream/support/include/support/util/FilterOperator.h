/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <vector>
#include <tuple>

namespace support {

    template <typename Filter, typename ...Ts>
    class FilterOperator {
    public:
        void operator()(Ts... args) {
            _filter(_result, args...);
        }

        std::vector<std::tuple<Ts...>> get() const {
            return _result;
        }

    private:
        std::vector<std::tuple<Ts...>> _result;
        Filter _filter;
    };

}  // namespace support