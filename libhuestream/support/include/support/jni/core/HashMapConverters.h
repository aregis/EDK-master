/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <unordered_map>

#include "support/jni/core/ConverterCore.h"
#include "support/jni/core/JNILocalRef.h"
#include "support/jni/core/HashMap.h"

extern template struct huesdk_jni_core::ClassInfo<huesdk_jni_core::HashMap>;

namespace huesdk_jni_core {
    template <typename T, typename U>
    struct from<std::unordered_map<T, U>> : from_base<std::unordered_map<T, U>> {
        explicit from(const std::unordered_map<T, U>& map) {
            auto env = JNIEnvFactory::Create();
            jmethodID constructor = env->GetMethodID(ClassInfo<HashMap>::Class, "<init>", "()V");
            this->_value = env->NewObject(ClassInfo<HashMap>::Class, constructor);

            for (auto&& elem : map) {
                call_java_method(&JNIEnv::CallObjectMethod, this->_value,
                                 "put", "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;", from<T>(elem.first), from<U>(elem.second));
            }
        }
    };

    template<typename T, typename U>
    struct to<std::unordered_map<T, U>> : to_base<std::unordered_map<T, U>> {
        explicit to(jobject instance) {
            auto key_set = call_java_method(&JNIEnv::CallObjectMethod, instance, "keySet", "()Ljava/util/Set;");
            auto key_array = static_cast<jobjectArray>(call_java_method(&JNIEnv::CallObjectMethod, key_set, "toArray", "()[Ljava/lang/Object;"));

            auto env = JNIEnvFactory::Create();
            auto size = env->GetArrayLength(key_array);
            auto cls = JNILocalRef<jclass>(env->GetObjectClass(instance));

            for (int64_t i = 0; i < size; ++i) {
                auto key = env->GetObjectArrayElement(key_array, i);

                jmethodID methodId = env->GetMethodID(cls.get(), "get", "(Ljava/lang/Object;)Ljava/lang/Object;");
                auto value = env.get()->CallObjectMethod(instance, methodId, key);
                this->_value[to<T>(key)] = to<U>(value);
            }
        }
    };
}  // namespace huesdk_jni_core