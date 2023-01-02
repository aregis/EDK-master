/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <utility>

#include "support/jni/SDKSupportJNI.h"
#include "support/jni/core/JNIEnvFactory.h"
#include "support/util/Exchange.h"

template <typename T>
class JNILocalRef {
public:
    JNILocalRef() : _localRef() {}

    explicit JNILocalRef(T localRef)
        : _localRef(localRef) {
    }

    JNILocalRef(const JNILocalRef&) = delete;

    JNILocalRef(JNILocalRef&& other) : JNILocalRef() {
        swap(*this, other);
    }

    ~JNILocalRef() {
        if (_localRef) {
            auto env = huesdk_jni_core::JNIEnvFactory::Create();

            if (env) {
                env->DeleteLocalRef(_localRef);
            }
        }
    }

    JNILocalRef& operator=(JNILocalRef other) {
        swap(*this, other);
        return *this;
    }

    T get() const {
        return _localRef;
    }

    T release() {
        return support::exchange(_localRef, nullptr);
    }

    explicit operator bool() const {
        return _localRef != nullptr;
    }

    bool operator==(const JNILocalRef& other) const {
        return other._localRef == _localRef;
    }

    bool operator!=(const JNILocalRef& other) const {
        return !(other == *this);
    }

    bool operator==(const T& other) const {
        return other == _localRef;
    }

    bool operator!=(const T& other) const {
        return !(*this == other);
    }

    friend void swap(JNILocalRef& a, JNILocalRef& b) {
        using std::swap;
        swap(a._localRef, b._localRef);
    }

    operator T() const {
        return _localRef;
    }

private:
    T _localRef = nullptr;
};

template<typename T>
bool operator==(const T& a, const JNILocalRef<T>& b) {
    return b == a;
}

template<typename T>
bool operator!=(const T& a, const JNILocalRef<T>& b) {
    return b != a;
}
