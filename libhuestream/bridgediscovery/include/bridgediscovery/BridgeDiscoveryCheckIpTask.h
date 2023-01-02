/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <memory>
#include <thread>
#include <string>

#include "support/threading/Job.h"

#include "bridgediscovery/BridgeDiscoveryResult.h"
#include "bridgediscovery/BridgeDiscoveryIpCheckResult.h"

namespace support {
    class HttpRequestError;
    class IHttpResponse;
}

namespace huesdk {

    class BridgeDiscoveryCheckIpTask : public support::JobTask {
    public:
        /**
         Constructor
         * @param input result to check
         */
        explicit BridgeDiscoveryCheckIpTask(const std::shared_ptr<BridgeDiscoveryResult> &input);

        /**
         @see Job.h
         */
        void execute(CompletionHandler completion_handler) override;

        /**
         Gets task execution result
         @return const reference to result struct
        */
        const BridgeDiscoveryIpCheckResult &get_result() const;

    private:
        void parse_config_response(support::HttpRequestError* error, support::IHttpResponse* response);

        std::shared_ptr<BridgeDiscoveryResult> _input;
        BridgeDiscoveryIpCheckResult _result;
    };

}  // namespace huesdk


