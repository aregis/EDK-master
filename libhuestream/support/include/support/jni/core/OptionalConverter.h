/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <jni.h>

#include <string>
#include <memory>
#include <utility>

#include <boost/optional.hpp>

#include "ConverterCore.h"
#include "Converter.h"

namespace huesdk_jni_core {

    template<typename T>
    struct from<boost::optional<T>> : from_base<boost::optional<T>> {
        template<typename S, typename = typename std::enable_if<std::is_convertible<S, T>::value>::type>
        explicit from(const boost::optional<S>& value) {
            if (value == boost::none) {
                this->_value = nullptr;
            } else {
                this->_value = from<S>(value.value()).value();
            }
        }

        explicit from(std::unique_ptr<T>&& value) {
            if (value == nullptr) {
                this->_value = nullptr;
            } else {
                this->_value = from<T>(std::move(value)).value();
            }
        }
    };

    template<typename T>
    struct to<boost::optional<T>> : to_base<boost::optional<T>> {
        explicit to(jobject value) {
            if (value == nullptr) {
                this->_value = boost::none;
            } else {
                this->_value = to<T>(value).value();
            }
        }
    };

    /* Note: from<boost::optional<>> is specialized for std::string because the type in
     * JNI is jstring instead of jobject
     */
    template<>
    struct from<boost::optional<std::string>> {
        using Target = jstring;

        explicit from(const boost::optional<std::string>& value) {
            if (value == boost::none) {
                this->_value = nullptr;
            } else {
                this->_value = from<std::string>(value.value()).value();
            }
        }

        Target value() const { return _value; }
        operator Target() const { return _value; }  // NOLINT
        Target _value;
    };

    /* Note: to<boost::optional<>> is specialized for std::string because the type in
     * JNI is jstring instead of jobject
     */
    template<>
    struct to<boost::optional<std::string>> : to_base<boost::optional<std::string>> {
        explicit to(jstring value) {
            if (value == nullptr) {
                this->_value = boost::none;
            } else {
                this->_value = to<std::string>(value).value();
            }
        }
        explicit to(jobject value) : to(static_cast<jstring>(value)) {}
    };

    /*
     * from<optional<>> and to<optional<>> are specialized below for primitives types
     * because those are boxed in Java when optional, while they are primitive when not.
     */

    template<>
    struct from<boost::optional<int>> : from_base<boost::optional<int>> {
        explicit from(const boost::optional<int>& value) {
            if (value == boost::none) {
                this->_value = nullptr;
            } else {
                auto env = JNIEnvFactory::Create();
                static jclass cls = static_cast<jclass>(env->NewGlobalRef(env->FindClass("java/lang/Integer")));
                static jmethodID methodID = env->GetMethodID(cls, "<init>", "(I)V");
                this->_value = env->NewObject(cls, methodID, value.value());
            }
        }
    };

    template<>
    struct to<boost::optional<int>> : to_base<boost::optional<int>> {
        explicit to(jobject value) {
            if (value == nullptr) {
                this->_value = boost::none;
            } else {
                auto env = JNIEnvFactory::Create();
                static jclass cls = static_cast<jclass>(env->NewGlobalRef(env->FindClass("java/lang/Integer")));
                static jmethodID methodID = env->GetMethodID(cls,  "intValue", "()I");
                this->_value = env->CallIntMethod(value, methodID);
            }
        }
    };

    template<>
    struct from<boost::optional<bool>> : from_base<boost::optional<bool>> {
        explicit from(const boost::optional<bool>& value) {
            if (value == boost::none) {
                this->_value = nullptr;
            } else {
                auto env = JNIEnvFactory::Create();
                static jclass cls = static_cast<jclass>(env->NewGlobalRef(env->FindClass("java/lang/Boolean")));
                static jmethodID methodID = env->GetMethodID(cls, "<init>", "(Z)V");
                this->_value = env->NewObject(cls, methodID, value.value());
            }
        }
    };

    template<>
    struct to<boost::optional<bool>> : to_base<boost::optional<bool>> {
        explicit to(jobject value) {
            if (value == nullptr) {
                this->_value = boost::none;
            } else {
                auto env = JNIEnvFactory::Create();
                static jclass cls = static_cast<jclass>(env->NewGlobalRef(env->FindClass("java/lang/Boolean")));
                static jmethodID methodID = env->GetMethodID(cls,  "booleanValue", "()Z");
                this->_value = env->CallBooleanMethod(value, methodID) != JNI_FALSE;
            }
        }
    };

}  // namespace huesdk_jni_core
