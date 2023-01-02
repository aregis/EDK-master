/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <jni.h>
#include <memory>
#include <functional>

namespace huesdk_jni_core {

    using JNIEnvUniquePtr = std::unique_ptr<JNIEnv, std::function<void(JNIEnv*)>>;

    struct JNIEnvFactory {
        static JavaVM* JVM;
        static const jint JNIVersion = JNI_VERSION_1_6;

        static JNIEnvUniquePtr Create() {
            JNIEnvUniquePtr return_value;

            if (JVM != nullptr) {
                JNIEnv* env = nullptr;
                int env_stat = JVM->GetEnv(reinterpret_cast<void**>(&env), JNIVersion);

                bool is_attached = false;
                switch (env_stat) {
                    case JNI_EDETACHED:
    #ifdef __ANDROID__
                        if (JVM->AttachCurrentThread(&env, 0) == 0) {
    #else
                        if (JVM->AttachCurrentThread((void**) &env, 0) == 0) {  // NOLINT
    #endif
                            is_attached = true;
                        } else {
                            env = nullptr;
                        }
                        break;
                    case JNI_OK:
                        break;
                    default:
                        env = nullptr;
                        break;
                }

                if (env) {
                    return_value = JNIEnvUniquePtr{env, [is_attached](JNIEnv*) {
                        if (is_attached) {
                            JVM->DetachCurrentThread();
                        }
                    }};
                }
            }

            return return_value;
        }
    };  // NOLINT

}  // namespace huesdk_jni_core

