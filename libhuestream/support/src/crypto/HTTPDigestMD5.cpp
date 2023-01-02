/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <string>

#include "support/logging/Log.h"
#include "support/crypto/HTTPDigestMD5.h"

#define HUESDK_LIB_HTTP_DIGEST_MD5_COLON ":"

using std::string;

namespace support {
    
    size_t HTTPDigestMD5::calculate_response(const char* username, const char* password, const char* realm, const char* method, const char* digest_uri, const char* nonce, char* buffer, size_t buffer_size) {
        if (buffer_size < HUESDK_LIB_HTTPDIGESTMD5_HTTP_HEX_DIGEST_SIZE) {
            return static_cast<size_t>(-1);
        }
        
        string credentials(Hash::md5(string(username) + HUESDK_LIB_HTTP_DIGEST_MD5_COLON + realm + HUESDK_LIB_HTTP_DIGEST_MD5_COLON + password, true));
        string resource(Hash::md5(string(method) + HUESDK_LIB_HTTP_DIGEST_MD5_COLON + digest_uri, true));

        string response(Hash::md5(credentials + HUESDK_LIB_HTTP_DIGEST_MD5_COLON + nonce + HUESDK_LIB_HTTP_DIGEST_MD5_COLON + resource, true));
        
        size_t length = response.copy(buffer, buffer_size);
        buffer[length] = '\0';
        
        return length+1;
    }
    
}  // namespace support
