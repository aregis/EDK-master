/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <jni.h>

#include <assert.h>

#include <utility>
#include <vector>

#include "JNIEnvFactory.h"

namespace huesdk_jni_core {

    class Callback {
    public:
        Callback(jobject callback, const char* name, const char* signature, const char** argumentClasses = nullptr)
                : _name(name), _signature(signature), _argumentClasses(argumentClasses), _exists(true) {
            assert(callback != nullptr);

            auto env = JNIEnvFactory::Create();
            if (env) {
                _callback = env->NewGlobalRef(callback);
            }
        }

        virtual ~Callback() {
            auto env = JNIEnvFactory::Create();
            if (env) {
                env->DeleteGlobalRef(_callback);
            }
        }

        static void checkOptionalOverrides(JNIEnv* env, huesdk_jni_core::Callback** callbackList, jobject object, int size) {
            jclass theClass = env->FindClass("java/lang/Class");
            jmethodID getDeclaredMethod = env->GetMethodID(theClass, "getDeclaredMethod",
                                                           "(Ljava/lang/String;[Ljava/lang/Class;)Ljava/lang/reflect/Method;");
            jclass object_class = env->GetObjectClass(object);

            for (int i = 0; i < size; i++) {
                jstring methodName = env->NewStringUTF(callbackList[i]->get_name());
                jobjectArray argumentClasses = callbackList[i]->get_argument_classes(env);
                jobject method = nullptr;
                method = env->CallObjectMethod(object_class, getDeclaredMethod, methodName, argumentClasses);
                if (env->ExceptionCheck() || method == nullptr) {
                    env->ExceptionClear();
                    callbackList[i]->invalidateExistence();
                }
            }
        }

        template<typename JNIApi>
        auto call(JNIEnv*, JNIApi api) const -> decltype(call_java_method(api, nullptr, nullptr, nullptr)) {
            return call_java_method(api, _callback, _name, _signature);
        }

        template<typename JNIApi, typename... Args>
        auto call(JNIEnv*, JNIApi api, Args... args) const -> decltype(call_java_method(api, nullptr, nullptr, nullptr, args...)) {
            return call_java_method(api, _callback, _name, _signature, args...);
        }

        const char* get_name() { return _name; }

        jobjectArray get_argument_classes(JNIEnv* env) {
            if (_argumentClasses == nullptr) {
                return NULL;
            }
            std::vector<jclass> argumentClassesNative;
            jclass theClass = env->FindClass("java/lang/Class");
             for (int i=0; _argumentClasses[i] != 0; i++) {
                 argumentClassesNative.push_back(env->FindClass(_argumentClasses[i]));
            }
            jobjectArray argumentClasses = env->NewObjectArray(static_cast<jsize>(argumentClassesNative.size()), theClass, nullptr);
            int j = 0;
            for (auto argumentClassNative : argumentClassesNative) {
                env->SetObjectArrayElement(argumentClasses, j, argumentClassNative);
                j++;
            }
            return argumentClasses;
        }

        void invalidateExistence() { _exists = false; }

        bool exists() {return _exists;}

    private:
        jobject _callback = nullptr;
        const char* _name = nullptr;
        const char* _signature = nullptr;
        const char** _argumentClasses = nullptr;
        bool _exists;
    };

}  // namespace huesdk_jni_core

