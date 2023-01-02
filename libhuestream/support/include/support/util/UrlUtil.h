/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <string>

namespace support {
    class UrlUtil {
    public:
        static std::string get_host(const std::string& url);
    };
}