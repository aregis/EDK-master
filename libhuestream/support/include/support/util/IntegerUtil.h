/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <stdint.h>

namespace support {
    
    typedef enum IntegerConversionResult {
        INTEGER_OK = 0,         // conversion is ok
        NO_INTEGER,             // can not convert
        NOT_ALL,                // only some first symbols from the string is converted
        OUT_OF_RANGE            // result integer is out of range
    } IntegerConversionResult;

    /**
     Convert string value to 32 bit integer
     @param value The value to be converted to the integer
     @param result Return the converted integer
     @return INTEGER_OK if conversion is ok
     */
    IntegerConversionResult to_int32(const char* value, int32_t* result);

    /**
     Convert string value to 32 bit unsigned integer
     @param value The value to be converted to the unsigned integer
     @param result Return the converted integer
     @return INTEGER_OK if conversion is ok
     */
    IntegerConversionResult to_unsigned_int32(const char* value, uint32_t* result);

    /**
     Convert string value to 64 bit integer
     @param value The value to be converted to the integer
     @param result Return the converted integer
     @return INTEGER_OK if conversion is ok
     */
    IntegerConversionResult to_int64(const char* value, int64_t* result);

    /**
     Convert string value to 64 bit unsigned integer
     @param value The value to be converted to the unsigned integer
     @param result Return the converted integer
     @return INTEGER_OK if conversion is ok
     */
    IntegerConversionResult to_unsigned_int64(const char* value, uint64_t* result);

    /**
     Generate a random integer between the given range
     @param min
     @param max
     */
    int random_integer(int min, int max);
    
}  // namespace support

