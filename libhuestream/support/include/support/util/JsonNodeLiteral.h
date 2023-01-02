/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include "libjson/libjson.h"

namespace json {
    namespace literals {
        inline JSONNode operator ""_json(const char *s, std::size_t)  { return libjson::parse(s); }
    }  // namespace literals
}  // namespace json