/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <jni.h>
#include <vector>
#include <functional>

#include "support/jni/core/Core.h"
#include "support/jni/core/JNIEnvFactory.h"
#include "support/jni/core/ClassInfo.h"
#include "support/jni/core/JNILocalRef.h"

using huesdk_jni_core::JNIEnvFactory;
using huesdk_jni_core::ClassInfoRegister;

JavaVM* JNIEnvFactory::JVM = nullptr;
jobject InternalScope = nullptr;

namespace huesdk_jni_core {

std::vector<std::function<void(JNIEnv*)>>& GetClassInitialzers() {
    static std::vector<std::function<void(JNIEnv*)>> class_initialzers;
    return class_initialzers;
}

jint Init(JavaVM* vm, void* /*pvt*/) {
    JNIEnvFactory::JVM = vm;

    auto env = JNIEnvFactory::Create();
    ClassInfoRegister::InitClasses(env.get());
    JNILocalRef<jclass> ScopeEnumClass{env->FindClass("com/philips/lighting/hue/sdk/wrapper/WrapperObject$Scope")};
    const auto field = env->GetStaticFieldID(ScopeEnumClass, "Internal", "Lcom/philips/lighting/hue/sdk/wrapper/WrapperObject$Scope;");
    InternalScope = env->NewGlobalRef(env->GetStaticObjectField(ScopeEnumClass, field));
    return JNIEnvFactory::JNIVersion;
}

void Release(JavaVM* /*vm*/, void* /*pvt*/) {
    auto env = JNIEnvFactory::Create();
    env->DeleteGlobalRef(InternalScope);
    JNIEnvFactory::JVM = nullptr;
}

}  // namespace huesdk_jni_core
