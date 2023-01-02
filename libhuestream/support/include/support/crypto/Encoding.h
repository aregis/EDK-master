/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <string>

namespace support {

    class Encoding {
    public:
        static std::string base64_encode(const std::string& data);

        static std::string base64_decode(const std::string& data);
        
        static std::string hex_encode(const std::string& data);
    };
    
}  // namespace support

