/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include "support/util/TimeUtil.h"

#ifdef ANDROID

#include <limits.h>
#if defined(__LP64__)
#include <time.h>
#else
#include <time64.h>
#endif

#endif

#ifdef _WIN32
#include <time.h>  // NOLINT(build/include) - CppLint does not see ifdefs
#endif

using std::unique_lock;

namespace support {

    // Initialize the time mutex
    mutex time_mutex;
    
    time_t timegm(tm* time_struct) {
        unique_lock<mutex> time_lock(time_mutex);
        
        #ifdef ANDROID
        
        // time_t is signed on android
       
        #if defined(__LP64__)
            #ifdef ARMEABI_V7A
                time_t result = timegm(time_struct);
            #else
                time_t result = ::timegm(time_struct);
            #endif
        #else
            time64_t result = timegm64(time_struct);

            static const time_t time_max = ~(1L << (sizeof(time_t) * CHAR_BIT - 1));
            static const time_t time_min =  (1L << (sizeof(time_t) * CHAR_BIT - 1));

            if (result < time_min || result > time_max) {
                // Invalid time struct
                return -1;
            }
        #endif
        
        return result;
        
        #else

            #ifdef _WIN32
                #ifdef __GNUC__
                    return timegm(time_struct);
                #else
                    return ::_mkgmtime(time_struct);
                #endif
            #else
                return ::timegm(time_struct);
            #endif

        #endif
    }
    
    time_t mktime(tm* time_struct) {
        unique_lock<mutex> time_lock(time_mutex);
        
        return ::mktime(time_struct);
    }

    tm gmtime(const time_t* time) {  // NOLINT
        unique_lock<mutex> time_lock(time_mutex);

        tm* result = ::gmtime(time);

        return result != nullptr ? *result : tm();
    }

    tm localtime(const time_t* time) {  // NOLINT
        unique_lock<mutex> time_lock(time_mutex);

        tm *result = ::localtime(time);

        return result != nullptr ? *result : tm();
    }
    
}  // namespace support
