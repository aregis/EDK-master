/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <map>
#include <memory>
#include <mutex>
#include <regex>
#include <string>
#include <utility>
#include <vector>

#include "support/network/NetworkInterface.h"
#include "support/util/VectorOperations.h"

namespace support {

    class NetworkConfiguration {
    public:
        static bool is_ssl_check_disabled();

        static void set_ssl_check_disabled(bool disabled);

        static bool get_reuse_connections();

        static void set_reuse_connections(bool reuse);

        static bool use_http2();

        static void set_use_http2(bool use);

        // returns a default list if the domain specified in the url is not "pinned"
        static std::vector<std::string> get_trusted_certificates(const std::string& url);

        static const std::vector<std::string>& get_root_certificates();

    private:
        static std::mutex _mutex;
        static bool _disable_ssl_check;
        static bool _reuse_connections;
        static bool _use_http2;
    };

}  // namespace support

