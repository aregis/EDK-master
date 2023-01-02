/*******************************************************************************
Copyright (C) 2020 Signify Holding
All Rights Reserved.
********************************************************************************/
#pragma once

#include <string>
#include <mbedtls/md5.h>
#include "support/crypto/Hash.h"
#include "support/crypto/Encoding.h"

namespace support
{

    class HashMD5
    {
    public:
        HashMD5() : _started(false) {}
        ~HashMD5() {}

        void Init() {
            mbedtls_md5_init(&_ctx);
            mbedtls_md5_starts(&_ctx);
            _started = true;
        }

        bool Active() {
            return _started;
        }

        void Update(const uint8_t* data, size_t dataSize) {
            if (_started) {
                mbedtls_md5_update(&_ctx, data, dataSize);
            }
        }

        std::string Finish(bool hex_encode = false) {
            _started = false;

            uint8_t hash[Hash::MD5_HASH_SIZE];
            mbedtls_md5_finish(&_ctx, hash);
            mbedtls_md5_free(&_ctx);

            std::string digest(reinterpret_cast<const char*>(hash), Hash::MD5_HASH_SIZE);

            return hex_encode ? Encoding::hex_encode(digest) : digest;
        }

    private:
        HashMD5(HashMD5&) {}
        mbedtls_md5_context _ctx;
        bool _started;
    };

}  // namespace support

