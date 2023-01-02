/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#define GET_METHOD_SIGNATURE(arguments_signature, return_type_signature)  "(" arguments_signature ")" return_type_signature
#define GET_ARRAY_SIGNATURE(class_signature)              JNI_JAVA_PRIMITIVE_ARRAY class_signature
#define GET_FULLY_QUALIFIED_CLASS_SIGNATURE(class_name)   "L" class_name ";"

// Java no function argument
#define JNI_JAVA_NO_ARGUMENT                              ""

// Java Primitives
#define JNI_JAVA_PRIMITIVE_BOOLEAN                        "Z"
#define JNI_JAVA_PRIMITIVE_BYTE                           "B"
#define JNI_JAVA_PRIMITIVE_CHAR                           "C"
#define JNI_JAVA_PRIMITIVE_SHORT                          "S"
#define JNI_JAVA_PRIMITIVE_INT                            "I"
#define JNI_JAVA_PRIMITIVE_LONG                           "J"
#define JNI_JAVA_PRIMITIVE_FLOAT                          "F"
#define JNI_JAVA_PRIMITIVE_DOUBLE                         "D"
#define JNI_JAVA_PRIMITIVE_VOID                           "V"
#define JNI_JAVA_PRIMITIVE_ARRAY                          "["

// JAVA Classes
#define JNI_JAVA_CLASS_COLLECTION                         "java/util/Collection"
#define JNI_JAVA_CLASS_LIST                               "java/util/List"
#define JNI_JAVA_CLASS_STRING                             "java/lang/String"
#define JNI_JAVA_CLASS_INTEGER                            "java/lang/Integer"
#define JNI_JAVA_CLASS_DOUBLE                             "java/lang/Double"
#define JNI_JAVA_CLASS_BOOLEAN                            "java/lang/Boolean"
#define JNI_JAVA_CLASS_DATE                               "java/util/Date"
#define JNI_JAVA_CLASS_MAP_ENTRY                          "java/util/Map$Entry"
#define JNI_JAVA_CLASS_RUNTIME_EXCEPTION                  "java/lang/RuntimeException"
#define JNI_JAVA_CLASS_OBJECT                             "java/lang/Object"
#define JNI_JAVA_CLASS_BYTE_BUFFER                        "java/nio/ByteBuffer"

// Java package names
#define JNI_SDK_PACKAGE_NAME_BASE                         "com/philips/lighting/hue/sdk/wrapper/"
#define JNI_SDK_PACKAGE_NAME_UTILITIES                    JNI_SDK_PACKAGE_NAME_BASE     "utilities/"            // com/philips/lighting/hue/sdk/wrapper/utilities/"

// Java wrapper class names
#define JNI_SDK_CLASS_NATIVE_TOOLS                        JNI_SDK_PACKAGE_NAME_UTILITIES "NativeTools"  // com/philips/lighting/hue/sdk/wrapper/utilities/NativeTools"
#define JNI_SDK_CLASS_NET_UTIL                            JNI_SDK_PACKAGE_NAME_UTILITIES "NetUtil"      // com/philips/lighting/hue/sdk/wrapper/utilities/NetUtil"

