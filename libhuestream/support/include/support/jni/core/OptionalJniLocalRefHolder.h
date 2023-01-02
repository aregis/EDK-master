/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <jni.h>

#include <type_traits>

#include "JNILocalRef.h"

/* The java code generator sometimes creates jni type local objects. Those objects can either
 * be java primitives (e.g. jint, jboolean) or jave reference types (e.g. jobject or jarray).
 * It is important to distinguish those cases, because only objects that are reference types have to be deleted
 * (via deleteLocalRef). This facility uses SFINAE to wrap objects, depending on their type, in JNILocalRefs */

namespace huesdk_jni_core {

    namespace detail {
        template <bool value>
        struct Not;

        template <>
        struct Not<true> : public std::false_type {};

        template <>
        struct Not<false> : public std::true_type {};

        template<typename T>
        struct IsJniRefType : std::is_base_of<_jobject, std::remove_pointer_t<T>> {};

        template <typename T, typename = void>
        class optional_ref_holder;

        template <typename T>
        class optional_ref_holder<T, std::enable_if_t<IsJniRefType<T>::value>> {
        public:
            using type = JNILocalRef<T>;

            explicit optional_ref_holder(const T& data) : _data{data} {}

            T get() const {
                return _data.get();
            }

        private:
            JNILocalRef<T> _data;
        };

        template <typename T>
        class optional_ref_holder<T, std::enable_if_t<Not<IsJniRefType<T>::value>::value>> {
        public:
            using type = T;

            explicit optional_ref_holder(const T& data) : _data{data} {}

            T get() const {
                return _data;
            }

        private:
            T _data;
        };
    }  // namespace detail

    template <typename T>
    auto make_optional_ref_holder(const T& data) {
        return detail::optional_ref_holder<T>(data);
    }

}  // namespace huesdk_jni_core