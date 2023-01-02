/*******************************************************************************
Copyright (C) 2019 Signify Holding
All Rights Reserved.
********************************************************************************/
#pragma once

#include <cassert>
#include <utility>

namespace support {
    template<typename T, typename... Args>
    void throw_exception(Args&&... args) {
#if !defined(_HAS_EXCEPTIONS) || _HAS_EXCEPTIONS == 1
        throw T(std::forward<Args>(args)...);
#else
        assert(false);
#endif
    }

    template <typename Method, typename... Ts>
    void call_and_ignore_exception(Method&& method, Ts&&... args) {
#if !defined(_HAS_EXCEPTIONS) || _HAS_EXCEPTIONS == 1
        try {
            method(args...);
        } catch (...) {
            // do nothing
        }
#else
        method(args...);
#endif
    }
}  // namespace support

