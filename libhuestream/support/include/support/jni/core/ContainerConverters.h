/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <memory>
#include <vector>
#include <set>

#include "Converter.h"
#include "ConverterCore.h"
#include "ClassInfo.h"
#include "Functional.h"
#include "ArrayList.h"
#include "TreeSet.h"
#include "JNILocalRef.h"

extern template struct huesdk_jni_core::ClassInfo<huesdk_jni_core::ArrayList>;
extern template struct huesdk_jni_core::ClassInfo<huesdk_jni_core::TreeSet>;

namespace huesdk_jni_core {

    template<typename T>
    struct from<std::vector<T>> : from_base<std::vector<T>> {
        explicit from(const std::vector<T>& value) {
            auto env = JNIEnvFactory::Create();
            jmethodID constructor = env->GetMethodID(ClassInfo<ArrayList>::Class, "<init>", "()V");
            this->_value = env->NewObject(ClassInfo<ArrayList>::Class, constructor);

            for (const auto& item : value) {
                call_java_method(&JNIEnv::CallBooleanMethod, this->_value, "add", "(Ljava/lang/Object;)Z", from<T>(item));
            }
        }
    };

    template<typename T>
    struct to<std::vector<T>> : to_base<std::vector<T>> {
        explicit to(jobject instance) {
            auto size = call_java_method(&JNIEnv::CallIntMethod, instance, "size", "()I");

            for (int64_t i = 0; i < size; ++i) {
                auto object = call_java_method(&JNIEnv::CallObjectMethod, instance, "get", "(I)Ljava/lang/Object;", from<int64_t>(i));
                if (object != nullptr) {
                    this->_value.push_back(to<T>(object));
                }
            }
        }
    };

    template<typename T>
    struct from<std::shared_ptr<std::vector<T>>> : from_base<std::shared_ptr<std::vector<T>>> {
        explicit from(std::shared_ptr<std::vector<T>> value) {
            if (value) {
                this->_value = from<std::vector<T>>(*value);
            }
        }
    };

    template<typename T>
    struct to<std::shared_ptr<std::vector<T>>> : to_base<std::shared_ptr<std::vector<T>>> {
        explicit to(jobject instance) {
            this->_value.reset(new std::vector<T>{});
            *this->_value = to<std::vector<T>>(instance);
        }
    };

    template<typename T>
    struct from<std::set<T>> : from_base<std::set<T>> {
        explicit from(const std::set<T>& value) {
            auto env = JNIEnvFactory::Create();
            jmethodID constructor = env->GetMethodID(ClassInfo<TreeSet>::Class, "<init>", "()V");
            this->_value = env->NewObject(ClassInfo<TreeSet>::Class, constructor);

            for (const auto& item : value) {
                call_java_method(&JNIEnv::CallBooleanMethod, this->_value, "add", "(Ljava/lang/Object;)Z", from<T>(item));
            }
        }
    };

    template<typename T>
    struct to<std::set<T>> : to_base<std::set<T>> {
        explicit to(jobject instance) {
            auto size = call_java_method(&JNIEnv::CallIntMethod, instance, "size", "()I");

            for (int64_t i = 0; i < size; ++i) {
                auto object = call_java_method(&JNIEnv::CallObjectMethod, instance, "get", "(I)Ljava/lang/Object;", from<int64_t>(i));
                if (object != nullptr) {
                    this->_value.insert(to<T>(object));
                }
            }
        }
    };

    template<typename T>
    struct from<std::shared_ptr<std::set<T>>> : from_base<std::shared_ptr<std::set<T>>> {
        explicit from(std::shared_ptr<std::vector<T>> value) {
            if (value) {
                this->_value = from<std::vector<T>>(*value);
            }
        }
    };

    template<typename T>
    struct to<std::shared_ptr<std::set<T>>> : to_base<std::shared_ptr<std::set<T>>> {
        explicit to(jobject instance) {
            this->_value.reset(new std::set<T>{});
            *this->_value = to<std::set<T>>(instance);
        }
    };

}  // namespace huesdk_jni_core

