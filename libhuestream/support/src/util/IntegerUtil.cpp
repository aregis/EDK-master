/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include "support/util/IntegerUtil.h"

#include <limits>
#include <ctime>
#include <random>

using std::random_device;
using std::mt19937;
using std::uniform_int_distribution;

namespace support {

    template<class IntegerType>
    IntegerConversionResult to_integer(const char* value, IntegerType* result) {
        if (value == nullptr) {
            return NO_INTEGER;
        }
        // conversion
        char *endptr;
        errno = 0;
        int64_t integer_value = strtoll(value, &endptr, 0);
        // analyze result
        if (value == endptr) {
            return NO_INTEGER;
        }
        if (*endptr != '\0') {
            *result = static_cast<IntegerType>(endptr - value);
            return NOT_ALL;
        }
        // the converted value should be in the range
        if (errno == ERANGE || integer_value < std::numeric_limits<IntegerType>::min() || integer_value > std::numeric_limits<IntegerType>::max()) {
            return OUT_OF_RANGE;
        }
        *result = static_cast<IntegerType>(integer_value);
        return INTEGER_OK;
    }

    template<class IntegerType>
    IntegerConversionResult to_unsigned_integer(const char* value, IntegerType* result) {
        if (value == nullptr) {
            return NO_INTEGER;
        }
        // check if the first symbol is minus
        if (value[0] == '-') {
            return OUT_OF_RANGE;
        }
        // conversion
        char *endptr;
        errno = 0;
        uint64_t integer_value = strtoull(value, &endptr, 0);
        // analyze result
        if (value == endptr) {
            return NO_INTEGER;
        }
        if (*endptr != '\0') {
            *result = static_cast<IntegerType>(endptr - value);
            return NOT_ALL;
        }
        // the converted value should be in the range
        if (errno == ERANGE || integer_value < std::numeric_limits<IntegerType>::min() || integer_value > std::numeric_limits<IntegerType>::max()) {
            return OUT_OF_RANGE;
        }
        *result = static_cast<IntegerType>(integer_value);
        return INTEGER_OK;
    }

    IntegerConversionResult to_int32(const char* value, int32_t* result) {
        return to_integer<int32_t>(value, result);
    }

    IntegerConversionResult to_int64(const char* value, int64_t* result) {
        return to_integer<int64_t>(value, result);
    }

    IntegerConversionResult to_unsigned_int32(const char* value, uint32_t* result) {
        return to_unsigned_integer<uint32_t>(value, result);
    }

    IntegerConversionResult to_unsigned_int64(const char* value, uint64_t* result) {
        return to_unsigned_integer<uint64_t>(value, result);
    }

    int random_integer(int min, int max) {
        random_device rd;
        mt19937 mt(rd());
        uniform_int_distribution<int> dist(min, max);

        return dist(mt);
    }

}  // namespace support
