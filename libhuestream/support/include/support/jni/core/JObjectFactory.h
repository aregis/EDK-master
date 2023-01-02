/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <memory>
#include <utility>

#include "ClassInfo.h"
#include "Functional.h"

extern jobject InternalScope;

namespace huesdk_jni_core {

    template <typename T>
    jobject create_jobject() {
        auto env = JNIEnvFactory::Create();
        jmethodID constructor = env->GetMethodID(ClassInfo<T>::Class, "<init>", "(Lcom/philips/lighting/hue/sdk/wrapper/WrapperObject$Scope;)V");
        if (constructor == nullptr) {
            JNIEnvFactory::Create()->ExceptionClear();
            return nullptr;
        }
        return env->NewObject(ClassInfo<T>::Class, constructor, InternalScope);
    }

    template <typename T>
    jobject create_jobject(T* value, bool takeOwnership) {
        if (value == nullptr) {
            return NULL;
        }

        auto return_value = create_jobject<T>();
        if (return_value) {
            set_reference(return_value, value, takeOwnership);
        }
        return return_value;
    }

    template <typename T>
    jobject create_jobject(std::shared_ptr<T> value) {
        if (value == nullptr) {
            return NULL;
        }

        auto return_value = create_jobject<T>();
        if (return_value) {
            set_reference(return_value, value);
        }
        return return_value;
    }

    template <typename T>
    jobject create_jobject(std::unique_ptr<T> value) {
        if (value == nullptr) {
            return NULL;
        }

        auto return_value = create_jobject<T>();
        if (return_value) {
            set_reference(return_value, std::move(value));
        }
        return return_value;
    }

}  // namespace huesdk_jni_core

