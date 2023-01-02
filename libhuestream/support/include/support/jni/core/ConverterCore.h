/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <string>

namespace huesdk_jni_core {

    template<typename T>
    struct from {
    };

    template<typename T>
    struct to {
    };

    template<typename T>
    struct from_base {
        using Target = jobject;

        Target value() const { return _value; }
        operator Target() const { return _value; }  // NOLINT

        Target _value = nullptr;
    };

    template<typename T>
    struct to_base {
        using Target = T;

        Target value() const & { return _value; }
        operator Target() const & { return _value; }  // NOLINT

        Target value() && { return std::move(_value); }
        operator Target() && { return std::move(_value); }  // NOLINT

        Target _value = {};
    };

    template <typename From, typename To>
    To convert(const From& value);

    template<typename From, typename To>
    struct base_converter {
        base_converter<From, To>(const From& value) : _value(convert<From, To>(value)) {}
        To value() const { return _value; }
        operator To() const { return _value; }
        To _value;
    };

    template<>
    struct to<void*> : base_converter<int64_t, void*>{                     // NOLINT
        using base_converter<int64_t, void*>::base_converter;              // NOLINT
    };

    template<>
    struct from<void*> {
        using Target = int64_t;                                              // NOLINT
        template<typename T>
        from<void*>(T *ptr) : _value(reinterpret_cast<int64_t>(ptr)) {}     // NOLINT
        Target value() const { return _value; }
        Target _value;
    };

    template <>
    struct to<std::string> : base_converter<jstring, std::string> {
        explicit to(jobject instance) : base_converter(static_cast<jstring>(instance)) {}
    };

    template <>
    struct from<std::string> : base_converter<std::string, jstring> {
        using base_converter<std::string, jstring>::base_converter;
    };

}  // namespace huesdk_jni_core

#define DEFINE_CONVERTER(T1, T2) \
    template<> \
    struct to<T1> : base_converter<T2, T1> { \
        using base_converter<T2, T1>::base_converter; \
    }; \
    template<> \
    struct from<T1> : base_converter<T1, T2> { \
        using base_converter<T1, T2>::base_converter; \
    }

#define DEFINE_DEFAULT_CONVERT_FUNCTION(T1, T2) \
    template<> \
    T2 convert<T1, T2>(const T1& value) { \
        return static_cast<T2>(value); \
    }

