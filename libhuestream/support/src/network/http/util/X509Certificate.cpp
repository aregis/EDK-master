/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <mbedtls/x509_crt.h>
#include <mbedtls/oid.h>

#include <regex>

#include "support/network/http/util/X509Certificate.h"
#include "support/crypto/Encoding.h"

namespace support {

    const std::string begin_certificate = "-----BEGIN CERTIFICATE-----\n";
    const std::string end_certificate = "-----END CERTIFICATE-----\n";

    std::string X509Certificate::der_to_pem(const unsigned char* data, size_t len) {
        auto encoded = Encoding::base64_encode(std::string(reinterpret_cast<const char*>(data), len));
        return begin_certificate + encoded + end_certificate;
    }
    
    std::string X509Certificate::get_pem_data(const std::string& pem) {
        auto pem_search = pem;
        pem_search.erase(std::remove(pem_search.begin(), pem_search.end(), '\n'), pem_search.end());

        static std::regex certificate_regex(".*?-----BEGIN CERTIFICATE-----?([\\w\\W]*?)-----END CERTIFICATE-----.*?");

        std::smatch what;
        std::string pem_data;
        if (std::regex_match(pem_search, what, certificate_regex)) {
            pem_data = what[1].str();
        }

        return pem_data;
    }

    std::string X509Certificate::extract_pem(const std::string& certificate) {
        static std::regex certificate_regex(".*?(-----BEGIN CERTIFICATE-----[\\w\\W]*?-----END CERTIFICATE-----).*?");

        std::smatch what;
        std::string certificate_content;
        if (std::regex_search(certificate, what, certificate_regex)) {
            certificate_content = std::string(what[0].first, what[0].second);
        }
        return certificate_content;
    }

    bool X509Certificate::are_equal(const std::string& pem1, const std::string& pem2) {
        mbedtls_x509_crt cert1;
        mbedtls_x509_crt cert2;

        mbedtls_x509_crt_init(&cert1);
        mbedtls_x509_crt_init(&cert2);

        mbedtls_x509_crt_parse(&cert1, reinterpret_cast<const unsigned char*>(pem1.c_str()), pem1.size() + 1);
        mbedtls_x509_crt_parse(&cert2, reinterpret_cast<const unsigned char*>(pem2.c_str()), pem2.size() + 1);

        bool equal = (cert1.raw.len == cert2.raw.len) && (memcmp(cert1.raw.p, cert2.raw.p, cert1.raw.len) == 0);

        mbedtls_x509_crt_free(&cert1);
        mbedtls_x509_crt_free(&cert2);

        return equal;
    }

    bool X509Certificate::check_common_name(const mbedtls_x509_crt* cert, const std::string& common_name) {
        const mbedtls_asn1_named_data* name = &cert->subject;

        while (name != nullptr) {
            if (MBEDTLS_OID_CMP(MBEDTLS_OID_AT_CN, &name->oid) == 0) {
                if (name->val.len == common_name.size()) {
                    if (memcmp(name->val.p, common_name.c_str(), common_name.size()) == 0) {
                        return true;
                    }
                }
            }
            name = name->next;
        }
        return false;
    }

}  // namespace support
