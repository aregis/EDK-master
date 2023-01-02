/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <jni.h>

#include "support/jni/SDKSupportJNI.h"

class HueJNIEnv {
public:
    HueJNIEnv()
            : _env(support::jni::getJNIEnv(&_has_attached)) {
    }

    ~HueJNIEnv() {
        if (_has_attached) {
            support::jni::getJavaVM()->DetachCurrentThread();
        }
    }

    operator bool() const {
        return _env != nullptr;
    }

    operator JNIEnv*() const {
        return _env;
    }

    JNIEnv* operator->() const {
        return _env;
    }

private:
    bool _has_attached = false;
    JNIEnv* _env = nullptr;
};

