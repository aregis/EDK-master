/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <mbedtls/base64.h>

#include <cassert>
#include <string>
#include <sstream>

#include "support/crypto/Encoding.h"

namespace support {

    std::string Encoding::base64_encode(const std::string& data) {
        size_t dlen;
        size_t slen = data.size();

        // get the size of the encoded string
        mbedtls_base64_encode(nullptr, 0, &dlen, nullptr, slen);

        unsigned char* dst = new unsigned char[dlen];
        const unsigned char* src = reinterpret_cast<const unsigned char*>(data.c_str());

        mbedtls_base64_encode(dst, dlen, &dlen, src, slen);

        std::string encoded = std::string(reinterpret_cast<const char*>(dst), dlen) + "\n";
        delete [] dst;
        return encoded;
    }

    std::string Encoding::base64_decode(const std::string& data) {
        size_t dlen;
        size_t slen = data.size();

        auto src = reinterpret_cast<const unsigned char*>(data.c_str());

        // get the size of the decoded string
        auto ret = mbedtls_base64_decode(nullptr, 0, &dlen, src, slen);
        if (ret == MBEDTLS_ERR_BASE64_INVALID_CHARACTER) {
            return "";
        }

        std::string decoded;
        auto dst = new unsigned char[dlen];

        ret = mbedtls_base64_decode(dst, dlen, &dlen, src, slen);
        if (ret == 0) {
            decoded = std::string(reinterpret_cast<const char*>(dst), dlen);
        }

        delete [] dst;
        return decoded;
    }

    std::string Encoding::hex_encode(const std::string& data) {
        size_t data_size = data.length();
        std::ostringstream hex;
        
        for (size_t i = 0; i < data_size; i++) {
            char j = (data[i] >> 4) & 0xf;
            if (j <= 9)
                hex << char(j + '0');
            else
                hex << char(j + 'a' - 10);
            j = data[i] & 0xf;
            if (j <= 9)
                hex << char(j + '0');
            else
                hex << char(j + 'a' - 10);
        }
        return hex.str();
    }

}  // namespace support
