/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include "Functional.h"
#include "ClassInfo.h"
#include "Converter.h"
#include "ObjectConverters.h"
#include "SmartPointerConverters.h"
#include "ContainerConverters.h"
#include "HashMapConverters.h"
#include "Callback.h"
#include "EnumSetConverter.h"
#include "OptionalConverter.h"

namespace huesdk_jni_core {

    jint Init(JavaVM* vm, void* pvt);
    void Release(JavaVM* vm, void* pvt);

}  // namespace huesdk_jni_core

using huesdk_jni_core::call_java_method;
using huesdk_jni_core::set_reference;
using huesdk_jni_core::delete_referenced_object;
using huesdk_jni_core::call;
using huesdk_jni_core::call_and_cache;
using huesdk_jni_core::from;
using huesdk_jni_core::to;
using huesdk_jni_core::EnumMapping;
using huesdk_jni_core::create_jobject;

