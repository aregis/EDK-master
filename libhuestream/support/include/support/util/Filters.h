/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <tuple>
#include <vector>
#include <utility>

#include "support/util/FilterOperator.h"

namespace support {

    namespace detail {
        struct First {
            template <typename... Ts>
            void operator()(std::vector<std::tuple<Ts...>>& result, Ts... args) const {
                if (result.size() == 0) {
                    result.emplace_back(args...);
                }
            }
        };

        struct Latest {
            template <typename... Ts>
            void operator()(std::vector<std::tuple<Ts...>>& result, Ts... args) const {
                result.clear();
                result.emplace_back(args...);
            }
        };

        struct Unique {
            template <typename... Ts>
            void operator()(std::vector<std::tuple<Ts...>>& result, Ts... args) const {
                auto current_arguments = std::make_tuple(args...);
                auto iter = std::find(result.begin(), result.end(), current_arguments);
                if (iter == result.end()) {
                    result.push_back(std::move(current_arguments));
                }
            }
        };
    }  // namespace detail

    template <typename... Ts>
    using FilterUnique = FilterOperator<support::detail::Unique, Ts...>;

    template <typename... Ts>
    using FilterFirst = FilterOperator<support::detail::First, Ts...>;

    template <typename... Ts>
    using FilterLatest = FilterOperator<support::detail::Latest, Ts...>;;

}  // namespace support
