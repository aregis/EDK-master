/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <functional>
#include <unordered_map>
#include <memory>
#include <string>
#include <utility>

#include "Proxy.h"
#include "Converter.h"

namespace huesdk_jni_core {

    template<typename JNIApi, typename... Args>
    auto call_java_method(JNIApi api, jobject instance, const char *method, const char *signature, Args... args)
        -> decltype((static_cast<JNIEnv*>(nullptr)->*api)(nullptr, nullptr, args.value()...)) {
        auto env = JNIEnvFactory::Create();
        auto jclass = env->GetObjectClass(instance);
        jmethodID methodId = env->GetMethodID(jclass, method, signature);
        env->DeleteLocalRef(jclass);
        return (env.get()->*api)(instance, methodId, args.value()...);
    }

    template <typename T>
    Proxy<T>* get_proxy(jobject instance) {
        return instance ? reinterpret_cast<Proxy<T>*>(call_java_method(&JNIEnv::CallLongMethod, instance, "getReference", "()J")) : nullptr;
    }

    template<typename T>
    T *get_reference(jobject instance) {
        auto proxy = get_proxy<T>(instance);
        return proxy ? proxy->subject() : nullptr;
    }

    template<typename T>
    void delete_referenced_object(jobject instance) {
        auto proxy = get_proxy<T>(instance);
        if (proxy != nullptr) {
            delete proxy;
            proxy = nullptr;
            call_java_method(&JNIEnv::CallVoidMethod, instance, "setReference", "(J)V", from<void *>(proxy));
        }
    }

    template<typename T>
    void set_reference(jobject instance, T *obj, bool takeOwnership) {
        delete_referenced_object<T>(instance);
        call_java_method(&JNIEnv::CallVoidMethod, instance, "setReference", "(J)V",
                         from<void*>(new Proxy<T>(obj, takeOwnership)));
    }

    template<typename T>
    void set_reference(jobject instance, std::shared_ptr<T> obj) {
        delete_referenced_object<T>(instance);
        call_java_method(&JNIEnv::CallVoidMethod, instance, "setReference", "(J)V",
                         from<void*>(new Proxy<T>(obj)));
    }

    template<typename T>
    void set_reference(jobject instance, std::unique_ptr<T> obj) {
        delete_referenced_object<T>(instance);
        call_java_method(&JNIEnv::CallVoidMethod, instance, "setReference", "(J)V",
                         from<void*>(new Proxy<T>(std::move(obj))));
    }

    template<typename Class, typename Method, typename... Args>
    auto call(jobject instance, Method method, Args... args) -> decltype(std::bind(method, get_proxy<Class>(instance)->subject(),
                                                                                   std::forward<Args>(args)...)()) {
        auto proxy = get_proxy<Class>(instance);
        return std::bind(method, proxy->subject(), std::forward<Args>(args)...)();
    };

    template<typename Class, typename Method, typename... Args>
    auto call(Method method, Args... args) -> decltype(std::bind(method, std::forward<Args>(args)...)()) {
        return std::bind(method, std::forward<Args>(args)...)();
    };

    template<typename Class, typename Method, typename... Args>
    jobject
    call_and_cache(const std::string &cache_key, jobject instance, Method method, Args... args) {
        jobject return_value = nullptr;

        auto env = JNIEnvFactory::Create();
        Proxy<Class> *proxy = get_proxy<Class>(instance);
        if (proxy != nullptr && env) {
            return_value = proxy->get_cache_item(cache_key, [&]() -> jobject {
                return from<decltype(std::bind(method, proxy->subject(), std::forward<Args>(args)...)())>(
                        std::bind(method, proxy->subject(), std::forward<Args>(args)...)());
            });
        }

        return return_value;
    }

}  // namespace huesdk_jni_core

