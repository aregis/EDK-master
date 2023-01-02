/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <vector>

#include "boost/range/begin.hpp"
#include "boost/range/end.hpp"
#include "boost/range/value_type.hpp"

namespace support {

    /* This takes a (possibly non-owning lazy) 2D range, flattens it, and produces a vector.
     * Note that this could be generalized to return a flattened range iso of a vector */

    template<typename Range2D>
    std::vector<typename boost::range_value<typename boost::range_value<Range2D>::type>::type>
    flatten(const Range2D& range) {
        using value_type = typename boost::range_value<typename boost::range_value<Range2D>::type>::type;
        std::vector<value_type> result;

        for (auto&& sub_range : range) {
            result.insert(std::end(result), boost::begin(sub_range), boost::end(sub_range));
        }
        return result;
    }

}  // namespace support
