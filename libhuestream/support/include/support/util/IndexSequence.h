/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <cstddef>

namespace support {

    template<std::size_t...> struct index_sequence {};

    template <std::size_t ...Ns> struct make_index_sequence;

    template <std::size_t I, std::size_t ...Ns>
    struct make_index_sequence<I, Ns...> {
        using type = typename make_index_sequence<I - 1, I - 1, Ns...>::type;
    };

    template <std::size_t ...Ns>
    struct make_index_sequence<0, Ns...> {
        using type = index_sequence<Ns...>;
    };

    template <std::size_t N>
    using make_index_sequence_t = typename make_index_sequence<N>::type;

}  // namespace support