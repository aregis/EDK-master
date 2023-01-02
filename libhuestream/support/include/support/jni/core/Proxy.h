/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <jni.h>
#include <utility>
#include <string>
#include <mutex>
#include <unordered_map>
#include <memory>

#include "JNIEnvFactory.h"

namespace huesdk_jni_core {

    template <typename T>
    std::unique_ptr<T> clone(const T& instance);

    template<typename T>
    class Proxy {
    public:
        Proxy<T>(T* subject, bool takeOwnership) : _subject(subject) {
            if (takeOwnership) {
                _shared_ptr.reset(subject);
            }
        }

        Proxy<T>(std::shared_ptr<T> subject) : _subject(subject.get()), _shared_ptr(subject) {}

        ~Proxy<T>() {
            auto env = JNIEnvFactory::Create();
            for (auto iter = _cache.begin(); iter != _cache.end(); ++iter) {
                env->DeleteGlobalRef(iter->second);
            }
        }

        T *subject() const {
            return _subject;
        }

        std::shared_ptr<T> take_shared_ptr() const {
            std::shared_ptr<T> return_value = _shared_ptr;

            if (return_value.get() == nullptr) {
                return_value = huesdk_jni_core::clone<T>(*_subject);
            }

            return return_value;
        }

        jobject get_cache_item(const std::string &key, std::function<jobject()> function) const {
            auto env = JNIEnvFactory::Create();
            std::lock_guard<std::mutex> lock{_cache_mutex};
            auto iter = _cache.find(key);
            if (iter == _cache.end()) {
                _cache[key] = env->NewGlobalRef(function());
            }

            return _cache[key];
        }

    private:
        T* _subject = nullptr;
        std::shared_ptr<T> _shared_ptr = nullptr;
        mutable std::mutex _cache_mutex;
        mutable std::unordered_map<std::string, jobject> _cache;
    };

}  // namespace huesdk_jni_core
