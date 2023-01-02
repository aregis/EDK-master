/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <time.h>
#include <mutex>

using std::mutex;
using std::tm;

 #ifndef _WIN32
    using std::time_t;
#endif

namespace support {
    
    /** ensure thread safety of the time functions */
    extern mutex time_mutex;
    
    /**
     The inverse of localtime and gmtime
     @return Seconds since epoch in UTC
     */
    extern time_t timegm(tm* time_struct);

    /**
     Thread safe extension of mktime
     @link http://www.cplusplus.com/reference/ctime/mktime/
     */
    extern time_t mktime(tm* time_struct);
    
    /**
     Thread safe extension of gmtime
     @link http://www.cplusplus.com/reference/ctime/gmtime/
     */
    extern tm gmtime(const time_t* time);  // NOLINT

    /**
     Thread safe extension of localtime
     @link http://www.cplusplus.com/reference/ctime/localtime/
     */
    extern tm localtime(const time_t* time);  // NOLINT
    
}  // namespace support

