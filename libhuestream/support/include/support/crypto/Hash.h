/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/
#pragma once

#include <string>

#define HUESDK_SHA256_DIGEST_LENGTH 32
#define HUESDK_MD5_DIGEST_LENGTH 16

namespace support {
    
    class Hash {
    public:
        /* SHA256 hashing */
        
        static const size_t SHA256_HASH_SIZE = HUESDK_SHA256_DIGEST_LENGTH;

        /**
         @param data Data to be hashed
         @param hex_encode return a hex-encoded digest if true
         @return SHA256 digest in binary or hex-encoded
         */
        static std::string sha256(const std::string& data, bool hex_encode = false);

        /* MD5 hashing */

        static const size_t MD5_HASH_SIZE = HUESDK_MD5_DIGEST_LENGTH;

        /**
         @param data Data to be hashed
         @param hex_encode return a hex-encoded digest if true
         @return MD5 digest in binary or hex-encoded
         */
        static std::string md5(const std::string& data, bool hex_encode = false);
    };
    
}  // namespace support

