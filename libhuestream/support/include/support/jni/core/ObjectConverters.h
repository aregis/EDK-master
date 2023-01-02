/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <memory>
#include <string>

#ifndef WRAPPER_DISABLE_LOG
#include "support/logging/Log.h"
#endif
#include "JObjectFactory.h"

#include "ConverterCore.h"

namespace huesdk_jni_core {

    template<typename T>
    struct from<T*> : from_base<T*> {
        from(T* value, bool takeOwnership) {
            this->_value = create_jobject(value, takeOwnership);
        }
    };

    template<typename T>
    struct to<T*> : to_base<T*> {
        explicit to(jobject value) {
            this->_value = get_reference<T>(value);
        }
    };

    template<typename T>
    struct from_base_object {
        using Target = jobject;
        from_base_object<T>(const T& value) {
            _value = create_jobject(std::make_shared<T>(value));
        }
        Target value() const { return _value; }
        operator Target() const { return _value; }
        Target _value;
    };

    template<typename T>
    struct to_base_object {
        using Target = T;
        to_base_object<T>(jobject value) {
            _value = get_reference<T>(value);
        }
        const Target& value() const { return *_value; }
        operator const Target&() const { return *_value; }
        Target* _value = nullptr;
    };

    template<typename T>
    struct to_base_enum {
        using Target = T;
        to_base_enum <T>(jobject value) {
            auto env = JNIEnvFactory::Create();
            const auto to_string = env->GetMethodID(EnumInfo<T>::Class, "toString", "()Ljava/lang/String;");
            const auto result = static_cast<jstring>(env->CallObjectMethod(value, to_string));
            const auto enum_name = env->GetStringUTFChars((jstring) result, 0);

            bool mapped = false;
            for (const auto& item : EnumInfo<T>::Mapping) {
                if (item.second == enum_name) {
                    _value = item.first;
                    mapped = true;
                    break;
                }
            }

            env->ReleaseStringUTFChars(result, enum_name);

            if (!mapped) {
#ifndef WRAPPER_DISABLE_LOG
                HUE_LOG << HUE_ERROR << HUE_WRAPPER << "Could not map java to base enum." << HUE_ENDL;
#endif
                std::terminate();
            }
        }
        const Target& value() const { return _value; }
        operator const Target&() const { return _value; }  // NOLINT
        Target _value;
    };

    template<typename T>
    struct from_base_enum {
        using Target = jobject;
        from_base_enum<T>(const T& value) {
            const auto& item = EnumInfo<T>::Mapping[value];
            if (item.size()) {
                auto env = JNIEnvFactory::Create();
                const auto field_type = std::string("L") + EnumInfo<T>::Name + ";";
                const auto field = env->GetStaticFieldID(EnumInfo<T>::Class, item.c_str(), field_type.c_str());
                _value = env->GetStaticObjectField(EnumInfo<T>::Class, field);
            } else {
#ifndef WRAPPER_DISABLE_LOG
                HUE_LOG << HUE_ERROR << HUE_WRAPPER << "Could not map base to java enum." << HUE_ENDL;
#endif
                std::terminate();
            }
        }
        Target value() const { return _value; }
        operator Target() const { return _value; }
        Target _value = nullptr;
    };

}  // namespace huesdk_jni_core

