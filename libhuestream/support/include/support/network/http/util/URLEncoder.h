/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <string>

namespace support {

    /**
     URL Encoder class - for internal use only
     */
    class URLEncoder {
    public:
        /**
         url encode function - for internal use only
         @param the std::string value to encode
         @return the urlencoded value as std::string
         */
        static std::string url_encode(std::string value);
    };
    
}  // namespace support

