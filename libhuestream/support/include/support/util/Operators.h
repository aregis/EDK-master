/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

namespace support {

    struct Or {
        template <typename T>
        T operator()(const T& lhs, const T& rhs) {
            return lhs | rhs;
        }
    };

    struct And {
        template <typename T>
        T operator()(const T& lhs, const T& rhs) {
            return lhs & rhs;
        }
    };

    struct Add {
        template <typename T>
        T operator()(const T& lhs, const T& rhs) {
            return lhs + rhs;
        }
    };

    struct Subtract {
        template <typename T>
        T operator()(const T& lhs, const T& rhs) {
            return lhs - rhs;
        }
    };

}  // namespace support