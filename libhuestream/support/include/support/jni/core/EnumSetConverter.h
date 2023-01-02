/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <vector>
#include <memory>

#include "support/util/EnumSet.h"

#include "Converter.h"
#include "ConverterCore.h"
#include "ClassInfo.h"
#include "Functional.h"

namespace huesdk_jni_core {

    template<typename T>
    struct from<support::EnumSet<T>> : from_base<support::EnumSet<T>> {
        explicit from(support::EnumSet<T> value) {
            auto env = JNIEnvFactory::Create();
            jmethodID jconverter = env->GetStaticMethodID(huesdk_jni_core::EnumInfo<T>::Class, "enumSetFromValue", "(J)Ljava/util/EnumSet;");
            this->_value = env->CallStaticObjectMethod(huesdk_jni_core::EnumInfo<T>::Class, jconverter, static_cast<int>(value));
        }
    };

    template<typename T>
    struct to<support::EnumSet<T>> : to_base<support::EnumSet<T>> {
        explicit to(jobject instance) {
            auto env = huesdk_jni_core::JNIEnvFactory::Create();
            jmethodID methodId = env->GetStaticMethodID(huesdk_jni_core::EnumInfo<T>::Class, "valueFromEnumSet",
                                                        "(Ljava/util/EnumSet;)J");
            auto v = env->CallStaticLongMethod(huesdk_jni_core::EnumInfo<T>::Class, methodId, instance);

            this->_value = support::EnumSet<T>(static_cast<int>(v));
        }
    };
}  // namespace huesdk_jni_core
