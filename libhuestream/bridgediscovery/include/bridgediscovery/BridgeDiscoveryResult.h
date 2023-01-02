/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <memory>
#include <string>

namespace huesdk {

    class BridgeDiscoveryResult {
    public:
        BridgeDiscoveryResult() = default;

        /**
         Constructor
         @param unique_id The unique id of the bridge
                    e.g. 0017880FFFE9B00A
         @param ip  The ip address of the bridge
                    e.g. 192.168.1.1 (only IPV4 supported for now)
         @param api_version  The api version of the bridge (e.g. 1.15.0)
         @param model_id The model ID of the bridge (BSB001 or BSB002)
         */
        BridgeDiscoveryResult(const std::string &unique_id, const std::string &ip, const std::string &api_version, const std::string &model_id, const std::string& name, const std::string& swversion);

        explicit BridgeDiscoveryResult(const std::string &ip);

        /**
         Get the unique id of the bridge
         @return the unique id or nullptr
         */
        const char *get_unique_id() const;

        /**
         Get ip address of the bridge
         @return the ip address or nullptr
         */
        const char *get_ip() const;

        /**
         Get api version of the bridge
         @return the api version
         */
        const char *get_api_version() const;

        /**
         Get the model ID of the bridge
         @return the model ID
         */
        const char *get_model_id() const;

        /**
         Get the name of the bridge
         @return the name
         */
        const char* get_name() const;

        /**
         Get the software version of the bridge
         @return the software version
         */
        const char* get_swversion() const;

        void set_unique_id(std::string);

        void set_ip(std::string);

        void set_api_version(std::string);

        void set_model_id(std::string);

    private:
        /** unique id of the bridge; format: 0017880FFFE9B00A */
        std::string _unique_id;
        /** ip address of the bridge;  format: 192.168.1.1 (only IPV4 supported for now) */
        std::string _ip;
        /** api version of the bridge;  format: 1.15.0 */
        std::string _api_version;
        /** model ID of the bridge;  format: BSB002 */
        std::string _model_id;
        /** name of the bridge */
        std::string _name;
        /** software version of the bridge; format: 1949107040 */
        std::string _swversion;
    };

}  // namespace huesdk

