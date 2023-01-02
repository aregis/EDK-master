/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <tuple>
#include <utility>

#include "boost/fusion/container/vector.hpp"
#include "boost/fusion/sequence.hpp"
#include "boost/mpl/int.hpp"

#include "support/util/IndexSequence.h"

namespace support {
    template <typename Operator, int N>
    struct tuple_for_each {
        template <typename... Ts>
        void operator()(std::tuple<Ts...>& result, const std::tuple<Ts...>& lhs, const std::tuple<Ts...>& rhs) {
            std::get<N>(result) = Operator{}(std::get<N>(lhs), std::get<N>(rhs));
            tuple_for_each<Operator, N - 1>{}(result, lhs, rhs);
        }
    };

    template <typename Operator>
    struct tuple_for_each<Operator, 0> {
        template <typename... Ts>
        void operator()(std::tuple<Ts...>& result, const std::tuple<Ts...>& lhs, const std::tuple<Ts...>& rhs) {
            std::get<0>(result) = Operator{}(std::get<0>(lhs), std::get<0>(rhs));
        }
    };

    // C++14 automatic return type deduction would greatly help increasing the readability here

    template<typename Callable, typename... Args, std::size_t... Indexes>
    auto apply_helper(Callable callable, index_sequence< Indexes... >, std::tuple<Args...>&& tup)
    -> decltype(callable(std::forward<Args>(std::get<Indexes>(tup))...)) {
        return callable(std::forward<Args>(std::get<Indexes>(tup))...);
    }

    template <typename Callable, typename ...Args>
    auto apply(Callable&& callable, const std::tuple<Args...>& tuple)
    -> decltype(apply_helper(callable, make_index_sequence_t<sizeof...(Args)>(), std::tuple<Args...>(tuple))) {
        using IndexSequence = make_index_sequence_t<sizeof...(Args)>;
        return apply_helper(callable, IndexSequence(), std::tuple<Args...>(tuple));
    }

    template<typename Callable, class ... Args>
    auto apply(Callable&& callable, std::tuple<Args...>&& tuple)
    -> decltype(apply_helper(callable, make_index_sequence_t<sizeof...(Args)>(), std::forward<std::tuple<Args...>>(tuple))) {
        using IndexSequence = make_index_sequence_t<sizeof...(Args)>;
        return apply_helper(callable, IndexSequence(), std::forward<std::tuple<Args...>>(tuple));
    }

    // boost fusion

    template<typename Callable, typename... Args, std::size_t... Indexes>
    auto apply_helper(Callable callable, index_sequence< Indexes... >, boost::fusion::vector<Args...>&& tup)
    -> decltype(callable(std::move(boost::fusion::at<boost::mpl::int_<Indexes>>(tup))...)) {
        return callable(std::move(boost::fusion::at<boost::mpl::int_<Indexes>>(tup))...);
    }

    template <typename Callable, typename ...Args>
    auto apply(Callable&& callable, const boost::fusion::vector<Args...>& tuple)
    -> decltype(apply_helper(callable, make_index_sequence_t<sizeof...(Args)>(), boost::fusion::vector<Args...>(tuple))) {
        using IndexSequence = make_index_sequence_t<sizeof...(Args)>;
        return apply_helper(callable, IndexSequence(), boost::fusion::vector<Args...>(tuple));
    }

    template<typename Callable, class ... Args>
    auto apply(Callable&& callable, boost::fusion::vector<Args...>&& tuple)
    -> decltype(apply_helper(callable, make_index_sequence_t<sizeof...(Args)>(), std::move(tuple))) {
        using IndexSequence = make_index_sequence_t<sizeof...(Args)>;
        return apply_helper(callable, IndexSequence(), std::move(tuple));
    }

}  // namespace support
