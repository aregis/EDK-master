/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <string>
#include <cstring>
#include <vector>

#include "support/jni/JNIToNative.h"
#include "support/jni/JNIConstants.h"
#include "support/jni/SDKSupportJNI.h"

#define TAG "JNIToNative"

using std::string;

namespace support {
    namespace jni {
        namespace util {

            bool isValidGlobalReference(JNIEnv* env, jobject globalRef) {
                if (globalRef != NULL) {
                    jobjectRefType refType = env->GetObjectRefType(globalRef);
                    if (refType == JNIGlobalRefType) {
                        return true;
                    }
                }
                return false;
            }

            /*
             * Convert a java array to a java list
             */
            jobject jArrayTojList(JNIEnv* env, jobjectArray j_array) {
                // use the java function in java
                jclass tools = g_cls_ref_tools;
                jmethodID mid = env->GetStaticMethodID(tools, "arrayToList",
                                                       GET_METHOD_SIGNATURE(GET_ARRAY_SIGNATURE(GET_FULLY_QUALIFIED_CLASS_SIGNATURE(JNI_JAVA_CLASS_OBJECT)), GET_FULLY_QUALIFIED_CLASS_SIGNATURE(JNI_JAVA_CLASS_LIST)));

                jobject list = env->CallStaticObjectMethod(tools, mid, j_array);
                return list;
            }

            jboolean boolean_N2J(bool value) {
                return value ? JNI_TRUE : JNI_FALSE;
            }

            bool boolean_J2N(jboolean value) {
                return (bool)(value == JNI_TRUE);  // NOLINT
            }

            const char* string_J2N(JNIEnv* env, jstring string_object) {
                // pass string object to java tools to get byte array
                jclass tools = g_cls_ref_tools;
                jmethodID mid = env->GetStaticMethodID(tools, "StringToBytes",
                                                       GET_METHOD_SIGNATURE(GET_FULLY_QUALIFIED_CLASS_SIGNATURE(JNI_JAVA_CLASS_STRING), GET_ARRAY_SIGNATURE(JNI_JAVA_PRIMITIVE_BYTE)));

                jobject j_array_object = env->CallStaticObjectMethod(tools, mid, string_object);
                if (j_array_object != NULL) {
                    jbyteArray j_array = static_cast<jbyteArray>(j_array_object);

                    const char* result = byteArrayToString(env, j_array);
                    env->DeleteLocalRef(j_array_object);
                    return result;
                } else {
                    LOGD("StringUtil", "byte array object was null");
                    return nullptr;
                }
            }

            jstring string_N2J(JNIEnv* env, const char* n_string) {
                if (n_string != nullptr) {
                    jbyteArray j_array = stringToByteArray(env, n_string);

                    // pass byte array to java tools to get java string object
                    jclass tools = g_cls_ref_tools;
                    jmethodID mid = env->GetStaticMethodID(tools, "BytesToString",
                                                           GET_METHOD_SIGNATURE(GET_ARRAY_SIGNATURE(JNI_JAVA_PRIMITIVE_BYTE), GET_FULLY_QUALIFIED_CLASS_SIGNATURE(JNI_JAVA_CLASS_STRING)));

                    jobject s_object = env->CallStaticObjectMethod(tools, mid, j_array);
                    // cleanup
                    env->DeleteLocalRef(j_array);

                    return static_cast<jstring>(s_object);
                } else {
                    return NULL;
                }
            }

            const char* byteArrayToString(JNIEnv* env, jbyteArray j_array) {
                if (j_array != NULL) {
                    // translate java byte array to const char*
                    int len = env->GetArrayLength(j_array);

                    // copy the array
                    char* buf = new char[len + 1];
                    env->GetByteArrayRegion(j_array, 0, len, reinterpret_cast<jbyte*>(buf));

                    // add null-terminator
                    buf[len] = '\0';

                    return buf;
                } else {
                    return nullptr;
                }
            }

            jbyteArray stringToByteArray(JNIEnv* env, const char* n_string) {
                if (n_string != nullptr) {
                    // get the length of the string
                    string str(n_string);
                    size_t len = str.length();

                    char* buffer = new char[len];
                    memcpy(buffer, n_string, len);

                    // translate const char* to java byte array
                    jbyteArray j_array = env->NewByteArray(static_cast<jsize>(len));
                    env->SetByteArrayRegion(j_array, 0, static_cast<jsize>(len), reinterpret_cast<jbyte*>(buffer));
                    delete[] buffer;
                    return j_array;
                } else {
                    return NULL;
                }
            }

            string jbyteArrayToString(JNIEnv* env, jobject parentObject, string fieldname) {
                string result = "";
                if (parentObject != NULL) {
                    jclass parentClass = env->GetObjectClass(parentObject);
                    jfieldID fid = env->GetFieldID(parentClass, fieldname.c_str(), GET_ARRAY_SIGNATURE(JNI_JAVA_PRIMITIVE_BYTE));
                    jbyteArray j_string = static_cast<jbyteArray>(env->GetObjectField(parentObject, fid));
                    env->DeleteLocalRef(parentClass);
                    if (j_string == NULL) {
                        return result;
                    }
                    const char* n_string = byteArrayToString(env, j_string);
                    result = string(n_string);
                    env->DeleteLocalRef((jobject)j_string);
                    delete[] n_string;
                }
                return result;
            }

            std::vector<std::string>* jstringArrayToStringVector(JNIEnv* env, jobject parentObject, string fieldname) {
                std::vector<std::string>* stringVector = new std::vector<std::string>();
                if (parentObject != NULL) {
                    // get class info of parent object
                    jobject j_arrayObject;
                    jclass j_parent_class = env->GetObjectClass(parentObject);
                    jfieldID fid = env->GetFieldID(j_parent_class, fieldname.c_str(),
                                                   GET_ARRAY_SIGNATURE(GET_FULLY_QUALIFIED_CLASS_SIGNATURE(JNI_JAVA_CLASS_STRING)));

                    if (fid == NULL) {
                        jmethodID mid = env->GetMethodID(j_parent_class, fieldname.c_str(),
                                                         GET_METHOD_SIGNATURE(JNI_JAVA_NO_ARGUMENT, GET_ARRAY_SIGNATURE(GET_FULLY_QUALIFIED_CLASS_SIGNATURE(JNI_JAVA_CLASS_STRING))));
                        if (mid == NULL) {
                            LOGE_EXT(TAG, "fatal error - cannot find field or method %s", fieldname.c_str());
                            return stringVector;
                        } else {
                            j_arrayObject = env->CallObjectMethod(parentObject, mid);
                        }
                    } else {
                        j_arrayObject = env->GetObjectField(parentObject, fid);
                    }

                    env->DeleteLocalRef(j_parent_class);

                    // retrieve the java string array
                    if (j_arrayObject == NULL) {
                        LOGD(TAG, "j array object null");
                        return stringVector;
                    }
                    jobjectArray j_stringArray = static_cast<jobjectArray>(j_arrayObject);
                    if (j_stringArray == NULL) {
                        LOGD(TAG, "string array null after cast");
                        return stringVector;
                    }

                    int j_stringArray_length = env->GetArrayLength(j_stringArray);
                    for (int i = 0; i < j_stringArray_length; i++) {
                        jstring j_string = static_cast<jstring>(env->GetObjectArrayElement(j_stringArray, i));
                        const char* n_string = string_J2N(env, j_string);
                        stringVector->push_back(string(n_string));
                        delete[] n_string;
                        LOGD_EXT(TAG, "reading string element %i", i);
                    }
                    env->DeleteLocalRef(j_stringArray);
                }
                LOGD_EXT(TAG, "string array size %i", stringVector->size());
                return stringVector;
            }
        }  // namespace util
    }  // namespace jni
}  // namespace support
