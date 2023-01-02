/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include "ConverterCore.h"

#include <jni.h>
#include <string>

namespace huesdk_jni_core {

    DEFINE_CONVERTER(int, jint);
    DEFINE_CONVERTER(int64_t, jlong);
    DEFINE_CONVERTER(bool, jboolean);
    DEFINE_CONVERTER(double, jdouble);
    DEFINE_CONVERTER(unsigned int, jint);

}  // namespace huesdk_jni_core

