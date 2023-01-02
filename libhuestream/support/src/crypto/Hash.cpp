/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <stddef.h>
#include <mbedtls/md5.h>
#include <mbedtls/sha256.h>

#include <string>

#include "support/crypto/Encoding.h"
#include "support/crypto/Hash.h"

namespace support {

    std::string Hash::sha256(const std::string& data, bool hex_encode) {
        unsigned char output[SHA256_HASH_SIZE];

        mbedtls_sha256(reinterpret_cast<const unsigned char*>(data.c_str()), data.size(), output, 0);

        std::string digest(reinterpret_cast<const char*>(output), SHA256_HASH_SIZE);

        return hex_encode ? Encoding::hex_encode(digest) : digest;
    }

    std::string Hash::md5(const std::string& data, bool hex_encode) {
        unsigned char output[MD5_HASH_SIZE];

        mbedtls_md5(reinterpret_cast<const unsigned char*>(data.c_str()), data.size(), output);

        std::string digest(reinterpret_cast<const char*>(output), MD5_HASH_SIZE);
        
        return hex_encode ? Encoding::hex_encode(digest) : digest;
    }
    
}  // namespace support
