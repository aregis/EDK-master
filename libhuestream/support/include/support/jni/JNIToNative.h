/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <jni.h>

#include <string>
#include <vector>

#include "support/util/VectorOperations.h"
#include "support/jni/JNIToNative.h"

namespace support {
    namespace jni {
        namespace util {

            bool isValidGlobalReference(JNIEnv* env, jobject globalRef);

            /*
             * integer conversion
             */
            jobject Integer_N2J(JNIEnv* env, int value);
            int     Integer_J2N(JNIEnv* env, jobject value);

            /*
             * boolean conversion
             */
            jboolean Boolean_N2J(bool value);
            bool     Boolean_J2N(jboolean value);

            /**
             * array conversion
             */
            jobject jArrayTojList(JNIEnv* env, jobjectArray j_array);

            /**
             * string conversion
             */
            const char* string_J2N(JNIEnv*, jstring);
            jstring     string_N2J(JNIEnv*, const char*);

            /**
             * string array conversion
             */
            std::string                      jbyteArrayToString(JNIEnv* env, jobject parentObject, std::string fieldname);
            const char*                      byteArrayToString(JNIEnv* env, jbyteArray j_array);
            jbyteArray                       stringToByteArray(JNIEnv* env, const char* n_string);
            std::vector<std::string>* jstringArrayToStringVector(JNIEnv* env, jobject parentObject, std::string fieldname);

        }  // namespace util
    }  // namespace jni
}  // namespace support

