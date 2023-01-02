/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <map>
#include <memory>
#include <string>
#include <regex>
#include <utility>

#include "support/network/NetworkConfiguration.h"

#define Q(x) #x
#define QUOTE(x) Q(x)

#include QUOTE(CERTIFICATE_MAP_FILE)

namespace {
    const std::vector<std::string> root_certificates = {
            /* root-bridge.cert.prime256v1.sha256.pem */
            R"(-----BEGIN CERTIFICATE-----
MIICMjCCAdigAwIBAgIUO7FSLbaxikuXAljzVaurLXWmFw4wCgYIKoZIzj0EAwIw
OTELMAkGA1UEBhMCTkwxFDASBgNVBAoMC1BoaWxpcHMgSHVlMRQwEgYDVQQDDAty
b290LWJyaWRnZTAiGA8yMDE3MDEwMTAwMDAwMFoYDzIwMzgwMTE5MDMxNDA3WjA5
MQswCQYDVQQGEwJOTDEUMBIGA1UECgwLUGhpbGlwcyBIdWUxFDASBgNVBAMMC3Jv
b3QtYnJpZGdlMFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAEjNw2tx2AplOf9x86
aTdvEcL1FU65QDxziKvBpW9XXSIcibAeQiKxegpq8Exbr9v6LBnYbna2VcaK0G22
jOKkTqOBuTCBtjAPBgNVHRMBAf8EBTADAQH/MA4GA1UdDwEB/wQEAwIBhjAdBgNV
HQ4EFgQUZ2ONTFrDT6o8ItRnKfqWKnHFGmQwdAYDVR0jBG0wa4AUZ2ONTFrDT6o8
ItRnKfqWKnHFGmShPaQ7MDkxCzAJBgNVBAYTAk5MMRQwEgYDVQQKDAtQaGlsaXBz
IEh1ZTEUMBIGA1UEAwwLcm9vdC1icmlkZ2WCFDuxUi22sYpLlwJY81Wrqy11phcO
MAoGCCqGSM49BAMCA0gAMEUCIEBYYEOsa07TH7E5MJnGw557lVkORgit2Rm1h3B2
sFgDAiEA1Fj/C3AN5psFMjo0//mrQebo0eKd3aWRx+pQY08mk48=
-----END CERTIFICATE-----
)",
            /* root-bridge.cert.secp521r1.sha512.pem */
            R"(-----BEGIN CERTIFICATE-----
MIICujCCAhugAwIBAgIUdLy1sTLm3TonVy03yWmRwxOo/kEwCgYIKoZIzj0EAwQw
OTELMAkGA1UEBhMCTkwxFDASBgNVBAoMC1BoaWxpcHMgSHVlMRQwEgYDVQQDDAty
b290LWJyaWRnZTAiGA8yMDE3MDEwMTAwMDAwMFoYDzIwMzgwMTE5MDMxNDA3WjA5
MQswCQYDVQQGEwJOTDEUMBIGA1UECgwLUGhpbGlwcyBIdWUxFDASBgNVBAMMC3Jv
b3QtYnJpZGdlMIGbMBAGByqGSM49AgEGBSuBBAAjA4GGAAQBXLIYniVZ+kPMf2Vh
jOB18bSHYSjrn9YhU1gSxcd9opSMKkV34Ps4B93nQlNbN7k0WbD7X5KNG/nSBm2T
NHB2BucB/NfA77vo7KYVdK9nenui2iAS4GIqYcXClG1l/8QTI1gOfRJUOAB6X2tw
bdgtb+KU9JtgInXvv2pUTlWs1qlLf7CjgbkwgbYwDwYDVR0TAQH/BAUwAwEB/zAO
BgNVHQ8BAf8EBAMCAYYwHQYDVR0OBBYEFDJ80OxBgte8rayRGJ7MtvJwBgvRMHQG
A1UdIwRtMGuAFDJ80OxBgte8rayRGJ7MtvJwBgvRoT2kOzA5MQswCQYDVQQGEwJO
TDEUMBIGA1UECgwLUGhpbGlwcyBIdWUxFDASBgNVBAMMC3Jvb3QtYnJpZGdlghR0
vLWxMubdOidXLTfJaZHDE6j+QTAKBggqhkjOPQQDBAOBjAAwgYgCQgF3HCwff4oR
3wObFZTXkLNeGSvISaHmrb1oJJfdl067YONc4OQ+z4/eJ/Ttdzduc45EK2+MXcYy
ilr+1jcqTJTzhwJCAeE7OE2k8SkznDcYAUljLE37vO3r9XCGJbqvkRgzWN4aI0fD
EiL8pLBintfYMdTpN1gQdAdecVKlysfs70A55vhh
-----END CERTIFICATE-----
)"
    };
}  // namespace

namespace support {
    std::mutex NetworkConfiguration::_mutex;

    bool NetworkConfiguration::_disable_ssl_check = false;

    bool NetworkConfiguration::_reuse_connections = true;

    bool NetworkConfiguration::_use_http2 = true;

    bool NetworkConfiguration::is_ssl_check_disabled() {
        return _disable_ssl_check;
    }

    void NetworkConfiguration::set_ssl_check_disabled(bool disabled) {
        _disable_ssl_check = disabled;
    }

    bool NetworkConfiguration::get_reuse_connections() {
        return _reuse_connections;
    }

    void NetworkConfiguration::set_reuse_connections(bool reuse) {
        _reuse_connections = reuse;
    }

    bool NetworkConfiguration::use_http2() {
        return _use_http2;
    }

    void NetworkConfiguration::set_use_http2(bool use) {
        _use_http2 = use;
    }

    std::vector<std::string> NetworkConfiguration::get_trusted_certificates(const std::string& url) {
        static std::regex url_regex("^https://([^/:]+)");

        std::smatch m;
        if (std::regex_search(url, m, url_regex)) {
            std::string domain_name = m[1].str();

            for (auto &item : default_certificate_mapping) {
                std::regex re = item.first;
                if (std::regex_match(domain_name, re)) {
                    return item.second;
                }
            }
        }

        return {};
    }

    const std::vector<std::string>& NetworkConfiguration::get_root_certificates() {
        return root_certificates;
    }

}  // namespace support
