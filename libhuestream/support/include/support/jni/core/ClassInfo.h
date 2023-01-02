/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <jni.h>
#include <functional>
#include <vector>
#include <map>
#include <string>

namespace huesdk_jni_core {

    template<typename Enum>
    using EnumMapping = std::map<Enum, std::string>;

    std::vector<std::function<void(JNIEnv*)>>& GetClassInitialzers();

    template<typename T>
    struct ClassInfo {
        static const char *Name;
        static jclass Class;
    };

    template <typename T>
    struct EnumInfo {
        using MappingT = EnumMapping<T>;
        static const char *Name;
        static jclass Class;
        static EnumMapping<T> Mapping;
    };

    class ClassInfoRegister {
    public:
        template<typename T>
        static void InitClass(JNIEnv *env) {
            ClassInfo<T>::Class = static_cast<jclass>(env->NewGlobalRef(
                    env->FindClass(ClassInfo<T>::Name)));
        }

        template<typename T>
        static void InitEnum(JNIEnv *env) {
            EnumInfo<T>::Class = static_cast<jclass>(env->NewGlobalRef(
                    env->FindClass(EnumInfo<T>::Name)));
        }

        template<typename Class>
        static const char *RegisterClass(const char *name) {
            GetClassInitialzers().push_back(std::bind(InitClass<Class>, std::placeholders::_1));
            return name;
        }

        template<typename Enum>
        static const char *RegisterEnum(const char *name) {
            GetClassInitialzers().push_back(std::bind(InitEnum<Enum>, std::placeholders::_1));
            return name;
        }

        static void InitClasses(JNIEnv *env) {
            const auto& container = GetClassInitialzers();
            for (const auto &initializer : container) {
                initializer(env);
            }
        }
    };

}  // namespace huesdk_jni_core

#define REGISTER_CLASS(CLASS, PATH) \
    template <> \
    const char* huesdk_jni_core::ClassInfo<CLASS>::Name = huesdk_jni_core::ClassInfoRegister::RegisterClass<CLASS>(PATH); \
    template <> \
    jclass huesdk_jni_core::ClassInfo<CLASS>::Class = nullptr;


#define REGISTER_ENUM(CLASS, PATH, MAPPING) \
    template <> \
    const char* huesdk_jni_core::EnumInfo<CLASS>::Name = huesdk_jni_core::ClassInfoRegister::RegisterEnum<CLASS>(PATH); \
    template <> \
    jclass huesdk_jni_core::EnumInfo<CLASS>::Class = nullptr; \
    template <> \
    typename huesdk_jni_core::EnumInfo<CLASS>::MappingT huesdk_jni_core::EnumInfo<CLASS>::Mapping = MAPPING;

#define DEFINE_WRAPPER(CLASS) \
    extern template struct huesdk_jni_core::ClassInfo<CLASS>; \
    template<> \
    struct huesdk_jni_core::to<CLASS> : huesdk_jni_core::to_base_object<CLASS> { \
        using huesdk_jni_core::to_base_object<CLASS>::to_base_object; \
    }; \
    template<> \
    struct huesdk_jni_core::from<CLASS> : huesdk_jni_core::from_base_object<CLASS> { \
        using huesdk_jni_core::from_base_object<CLASS>::from_base_object; \
    };

#define DEFINE_ENUM(ENUM) \
    extern template struct huesdk_jni_core::EnumInfo<ENUM>; \
    template<> \
    struct huesdk_jni_core::to<ENUM> : huesdk_jni_core::to_base_enum<ENUM> { \
        using huesdk_jni_core::to_base_enum<ENUM>::to_base_enum; \
    }; \
    template<> \
    struct huesdk_jni_core::from<ENUM> : huesdk_jni_core::from_base_enum<ENUM> { \
        using huesdk_jni_core::from_base_enum<ENUM>::from_base_enum; \
    };

