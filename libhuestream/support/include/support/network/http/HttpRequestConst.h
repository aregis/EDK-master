/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include "../../AttributeSpecifiers.h"

namespace support {

    /** default http connection timeout in seconds */
    const unsigned int HTTP_CONNECT_TIMEOUT                                = 10;
    /** default http request receive timeout in seconds */
    const unsigned int HTTP_RECEIVE_TIMEOUT                                = 30;
    /** default http request timeout in seconds (0 means indefinite) */
    const unsigned int HTTP_REQUEST_TIMEOUT                                = 300;
    /** predefined content types */
    static const char* HTTP_CONTENT_TYPE_APPLICATION_JSON                   MAYBE_USED = "application/json";
    static const char* HTTP_CONTENT_TYPE_MULTIPART_FORM_DATA                MAYBE_USED = "multipart/form-data";
    static const char* HTTP_CONTENT_TYPE_APPLICATION_X_WWW_FORM_URLENCODED  MAYBE_USED = "application/x-www-form-urlencoded";
    /* predefined headers */
    static const char* HTTP_HEADER_AUTHORIZATION                            MAYBE_USED = "Authorization";
    static const char* HTTP_HEADER_CONTENT_TYPE                             MAYBE_USED = "Content-Type";
    static const char* HTTP_HEADER_BEARER_AUTHENTICATION                    MAYBE_USED = "Bearer ";
    static const char* HTTP_HEADER_AUTHORIZATION_DIGEST_FORMAT              MAYBE_USED = "Digest username=\"%s\", realm=\"%s\", nonce=\"%s\", uri=\"%s\", response=\"%s\"";

    /** ciphers supported by api.meethue.com as of 2016-06-22 */
    static const char* TLS_CIPHER_WHITELIST[] MAYBE_USED = {
        "TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256",
        "TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA256",
        "TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA",
        "TLS_ECDHE_RSA_WITH_AES_256_GCM_SHA384",
        "TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA384",
        "TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA",
        "TLS_RSA_WITH_AES_128_GCM_SHA256",
        "TLS_RSA_WITH_AES_128_CBC_SHA256",
        "TLS_RSA_WITH_AES_128_CBC_SHA",
        "TLS_RSA_WITH_AES_256_GCM_SHA384",
        "TLS_RSA_WITH_AES_256_CBC_SHA256",
        "TLS_RSA_WITH_AES_256_CBC_SHA"
    };
    /** openssl version of the ciphers above */
    static const char* TLS_CIPHER_WHITELIST_OPENSSL MAYBE_USED =
        "ECDHE-RSA-AES128-GCM-SHA256:"
        "ECDHE-RSA-AES128-SHA256:"
        "ECDHE-RSA-AES128-SHA:"
        "ECDHE-RSA-AES256-GCM-SHA384:"
        "ECDHE-RSA-AES256-SHA384:"
        "ECDHE-RSA-AES256-SHA:"
        "AES128-GCM-SHA256:"
        "AES128-SHA256:"
        "AES128-SHA:"
        "AES256-GCM-SHA384:"
        "AES256-SHA256:"
        "AES256-SHA";

    static const unsigned int TLS_CIPHER_WHITELIST_LENGTH MAYBE_USED = sizeof(TLS_CIPHER_WHITELIST) / sizeof(TLS_CIPHER_WHITELIST[0]);

}  // namespace support

