/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <memory>
#include <string>
#include <vector>
#include <algorithm>
#include <numeric>
#include <mutex>
#include <cstdint>

#include <boost/iterator/counting_iterator.hpp>

#include "support/threading/OperationalQueue.h"
#include "support/jni/SDKSupportJNI.h"
#include "support/jni/JNIConstants.h"
#include "support/jni/core/JNILocalRef.h"
#include "support/jni/core/HueJNIEnv.h"

// logging tag
#define TAG "JNI_UTILITIES"

namespace support {
   namespace jni {
       jclass g_cls_ref_tools;
       jclass g_cls_ref_map_entry;
#if defined(ANDROID)
       jclass g_cls_ref_wifi_util_factory;
#endif

       std::string string_format(const char* fmt, ...) {
           // format the string
           int n = (static_cast<int>(strlen(fmt))) * 2; /* Reserve two times as much as the length of the fmt_str */
           std::string str;
           std::unique_ptr<char[]> formatted;
           va_list ap;
           while (1) {
               formatted.reset(new char[n]); /* Wrap the plain char array into the unique_ptr */
               strcpy(&formatted[0], fmt);  // NOLINT(runtime/printf)
               va_start(ap, fmt);
               int final_n = vsnprintf(&formatted[0], static_cast<size_t>(n), fmt, ap);
               va_end(ap);
               if (final_n < 0 || final_n >= n)
                   n += abs(final_n - n + 1);
               else
                   break;
           }
           return std::string(formatted.get());
       }

        // Global vars
        JavaVM* g_vm;  // global reference to JVM

        void init(JavaVM* vm) {
            g_vm = vm;

            JNIEnv* env = getJNIEnv();

            g_cls_ref_tools = [&]()->jclass {
                auto cls = JNILocalRef<jclass>{env->FindClass(JNI_SDK_CLASS_NATIVE_TOOLS)};
                if (cls) {
                    return static_cast<jclass>(env->NewGlobalRef(cls.get()));
                } else {
                    env->ExceptionClear();
                    return nullptr;
                }
            }();

            g_cls_ref_map_entry = [&] {
                auto cls = JNILocalRef<jclass>{env->FindClass(JNI_JAVA_CLASS_MAP_ENTRY)};
                return static_cast<jclass>(env->NewGlobalRef(cls.get()));
            }();

#if defined(ANDROID)
            g_cls_ref_wifi_util_factory = [&] {
                auto cls = JNILocalRef<jclass>{env->FindClass("com/philips/lighting/hue/sdk/wrapper/utilities/WifiUtilFactory")};
                return static_cast<jclass>(env->NewGlobalRef(cls.get()));
            }();
#endif
         }

       JavaVM *getJavaVM() {
            return g_vm;
        }

    JNIEnv *getJNIEnv(bool *has_attached) {
        JNIEnv *g_env;
#ifdef ANDROID
        JNIEnv ** j_env = &g_env;
#else
        void **j_env = (void **) &g_env;  // NOLINT(readability/casting)
#endif

        int getEnvStat = g_vm->GetEnv((void **) &g_env, JNI_VERSION_1_6);  // NOLINT(readability/casting)
        if (getEnvStat == JNI_EDETACHED) {
            if (g_vm->AttachCurrentThread(j_env, NULL) != 0) {
                LOGE(TAG, "Could not attach thread to vm!");
                return nullptr;
            } else {
                if (has_attached != nullptr) {
                    *has_attached = true;
                }
                return g_env;
            }
        } else if (getEnvStat == JNI_OK) {
            return g_env;
        } else if (getEnvStat == JNI_EVERSION) {
            // not supported
            LOGE(TAG, "JNI version does not support thread attaching - stop");
            return nullptr;
        }
        LOGE_EXT(TAG, "Unknown environment status: %i, - no JNIEnv", getEnvStat);
        return nullptr;  // unknown end
    }

       std::shared_ptr<QueueDispatcher> dispatcher() {
           static auto s_instance = std::make_shared<QueueDispatcher>(std::make_shared<OperationalQueue>());
           return s_instance;
       }

        int detect_available_local_refs(JNIEnv* env) {
            using Incrementable = int64_t;
            Incrementable i0 = 0;
            Incrementable i1 = 1024*1024*1024 + 1;
            boost::counting_iterator<Incrementable> begin(i0);
            boost::counting_iterator<Incrementable> end(i1);

            auto result = std::lower_bound(begin, end, 0, [env](const Incrementable& it, const Incrementable& /*value*/) {
                int jni_result = env->PushLocalFrame(static_cast<int>(it));
                if (jni_result == 0) {  // succeeded
                    env->PopLocalFrame(nullptr);
                } else {
                    env->ExceptionClear();
                }
                return jni_result >= 0;
            });

            if (result == end) {  // failed to find any index for which "env->PushLocalFrame(index)" succeeds
                LOGD(TAG, "detect_available_local_refs: failed, returning 0");
                return 0;
            }

            LOGD_EXT(TAG, "detect_available_local_refs: %i local references are available", static_cast<int>(*result) - 1);

            // result is the first index that fails at "env->PushLocalFrame(index)"
            return static_cast<int>(*result) - 1;
        }

    }  // namespace jni
}  // namespace support
