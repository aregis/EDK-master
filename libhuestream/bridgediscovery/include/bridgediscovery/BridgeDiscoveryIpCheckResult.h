/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <string>

namespace huesdk {

    struct BridgeDiscoveryIpCheckResult {
        /** the ip which has been checked */
        std::string ip;
        /** the unique id of the bridge */
        std::string unique_id;
        /** the api version of the bridge */
        std::string api_version;
        /** the model ID of the bridge */
        std::string model_id;
        /** whether the ip is reachable */
        bool reachable = false;
        /** whether the ip is a bridge */
        bool is_bridge = false;
        /** the name of the bridge */
        std::string name;
        /** the software version of the bridge */
        std::string swversion;
    };

}  // namespace huesdk
