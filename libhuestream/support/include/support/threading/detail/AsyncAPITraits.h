/*******************************************************************************
Copyright (C) 2019 Signify Holding
All Rights Reserved.
********************************************************************************/

#pragma once

#include <vector>

namespace support {
    template <typename T>
    class Future;
    template <typename T>
    class CompositeFuture;
    template <typename T>
    class Promise;

    namespace detail {
        template <class T> struct async_api_traits {
            using ReturnType = T;
        };

        template <class T> struct async_api_traits<Future<T>> {
            using ReturnType = T;
        };

        template <class T> struct async_api_traits<CompositeFuture<T>> {
            using ReturnType = std::vector<T>;
        };

        template <> struct async_api_traits<CompositeFuture<void>> {
            using ReturnType = void;
        };
    }  // namespace detail
}  // namespace support