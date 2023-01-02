/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <mbedtls/x509_crt.h>

#include <string>

namespace support {

    class X509Certificate {
    public:
        static std::string der_to_pem(const unsigned char*, size_t);

        static std::string get_pem_data(const std::string& pem);
                
        static std::string extract_pem(const std::string& certificate);

        static bool are_equal(const std::string& pem1, const std::string& pem2);

        /**
         * Checks the subject of a certificate against a common name
         * Wildcard matching is not supported
         * @return True if the certificate is issued for the specified common name
         */
        static bool check_common_name(const mbedtls_x509_crt* cert, const std::string& common_name);
    };

}  // namespace support
