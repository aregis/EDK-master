/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <utility>

#include "support/jni/SDKSupportJNI.h"
#include "support/jni/core/JNIEnvFactory.h"
#include "support/util/Exchange.h"

class JNIGlobalRef {
public:
    /**
     * @param jobj A reference of which ownership will be taken
     */
    explicit JNIGlobalRef(jobject jobj = nullptr) : _jobj(jobj) {}

    JNIGlobalRef(const JNIGlobalRef& other) : _jobj() {
        auto env = huesdk_jni_core::JNIEnvFactory::Create();
        if (env) {
            _jobj = env->NewGlobalRef(other._jobj);
        }
    }

    JNIGlobalRef(JNIGlobalRef&& other) : JNIGlobalRef() {
        swap(*this, other);
    }

    ~JNIGlobalRef() {
        if (_jobj) {
            auto env = huesdk_jni_core::JNIEnvFactory::Create();
            if (env) {
                env->DeleteGlobalRef(_jobj);
            }
        }
    }

    JNIGlobalRef& operator=(JNIGlobalRef other) {
        swap(*this, other);
        return *this;
    }

    /**
     * Release ownership of the jobject
     */
    jobject release() {
        return support::exchange(_jobj, nullptr);
    }

    /**
     * @param jobj A global reference of which ownership will be taken
     */
    void reset(jobject jobj) {
        *this = JNIGlobalRef(jobj);
    }

    jobject get() const {
        return _jobj;
    }

    explicit operator bool() const {
        return _jobj != nullptr;
    }

    bool operator==(const JNIGlobalRef& other) const {
        return other._jobj == _jobj;
    }

    bool operator!=(const JNIGlobalRef& other) const {
        return !(other == *this);
    }

    bool operator==(jobject other) const {
        return other == _jobj;
    }

    bool operator!=(jobject other) const {
        return !(*this == other);
    }

    friend void swap(JNIGlobalRef& a, JNIGlobalRef& b) {
        using std::swap;
        swap(a._jobj, b._jobj);
    }

private:
    jobject _jobj;
};

inline bool operator==(jobject a, const JNIGlobalRef& b) {
    return b == a;
}

inline bool operator!=(jobject a, const JNIGlobalRef& b) {
    return b != a;
}

/**
 * Creates a new global reference from jobj. No ownership of jobj is taken.
 */
inline JNIGlobalRef make_jni_global_ref(jobject jobj) {
    auto env = support::jni::getJNIEnv();
    if (env) {
        return JNIGlobalRef{env->NewGlobalRef(jobj)};
    } else {
        return JNIGlobalRef{};
    }
}
