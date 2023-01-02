/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
********************************************************************************/

#pragma once

#include <gmock/gmock.h>
#include "libjson/libjson.h"

namespace support {

    MATCHER_P(JsonNodeIsEqualTo, expected, "") {
        return libjson::parse(arg) == expected;
    }

}  // namespace support
