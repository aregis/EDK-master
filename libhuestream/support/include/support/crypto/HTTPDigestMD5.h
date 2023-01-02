/*******************************************************************************
Copyright (C) 2019 Signify Holding
All Rights Reserved.
********************************************************************************/
#pragma once

#include <string>

#include "support/crypto/Hash.h"

#define HUESDK_LIB_HTTPDIGESTMD5_HTTP_DIGEST_SIZE (HUESDK_MD5_DIGEST_LENGTH)
#define HUESDK_LIB_HTTPDIGESTMD5_HTTP_HEX_DIGEST_SIZE ((HUESDK_LIB_HTTPDIGESTMD5_HTTP_DIGEST_SIZE*2)+1)

namespace support {
    
    class HTTPDigestMD5 {
    public:
        /**
         calculates the HTTP Digest MD5 response:
            HA1 =      MD5( <username> + ":" +  <realm> + ":" + <password> )
            HA2 =      MD5(  <method>  + ":" + <password> )
            response = MD5(    HA1     + ":" +  <nonce> + ":" +    HA2 )
         @param username username
         @param password password
         @param realm    realm
         @param method   method (PUT, GET, POST, DELETE)
         @param digest_uri digest uri (e.g. /index.html)
         @param nonce    nonce
         @param buffer   buffer in which to store (readable) response
         @param buffer_size size of buffer
         @return size of (readable) response (should be of size HTTP_HEX_DIGEST_SIZE)
         */
        static size_t calculate_response(const char* username, const char* password, const char* realm, const char* method, const char* digest_uri, const char* nonce, char* buffer, size_t buffer_size);
    };
    
}  // namespace support

