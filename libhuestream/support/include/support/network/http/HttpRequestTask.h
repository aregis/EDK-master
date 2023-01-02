/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <string>
#include <memory>

#include "support/threading/Job.h"
#include "support/network/http/IHttpResponse.h"
#include "support/network/http/HttpRequestError.h"
#include "support/network/http/HttpRequestConst.h"

using Task = support::JobTask;
using support::IHttpResponse;
using support::HttpRequestError;

namespace support {
    class HttpRequestTask : public Task {
    public:
        struct Options {
            int connect_timeout = support::HTTP_CONNECT_TIMEOUT;
            int receive_timeout = support::HTTP_RECEIVE_TIMEOUT;
            int request_timeout = support::HTTP_REQUEST_TIMEOUT;
            bool use_proxy = false;
            std::string proxy_address = "";
            int proxy_port = 0;
            bool verify_ssl = true;
        };

        struct HttpRequest;

        explicit HttpRequestTask(const std::string& url, const Options* options = nullptr);

        void set_connect_timeout(int connect_timeout);
        void set_receive_timeout(int receive_timeout);
        void set_request_timeout(int request_timeout);
        void set_proxy(const std::string& proxy_address, int proxy_port);

        IHttpResponse* get_response() const;
        HttpRequestError* get_error() const;

        ~HttpRequestTask() override;

    protected:
        void execute(CompletionHandler completion_handler) override;
        void stop() override;

    private:
        std::unique_ptr<HttpRequestError> _error;
        std::unique_ptr<HttpRequest> _request;
        std::unique_ptr<IHttpResponse> _response;
    };
}  // namespace support


