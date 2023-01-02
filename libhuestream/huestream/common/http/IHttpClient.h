/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#ifndef HUESTREAM_COMMON_HTTP_IHTTPCLIENT_H_
#define HUESTREAM_COMMON_HTTP_IHTTPCLIENT_H_

#include <string>
#include <memory>

#include "huestream/common/http/HttpRequestInfo.h"
#include "support/network/http/HttpRequest.h"

namespace huestream {

    using HttpRequestCallback = support::HttpRequestCallback;

    class IHttpClient {
    public:
        struct HttpRequest;

        virtual ~IHttpClient() = default;

        virtual void Execute(HttpRequestPtr request) = 0;
        virtual int32_t ExecuteAsync(HttpRequestPtr request, HttpRequestCallback callback = {}) = 0;

        virtual void CancelAsyncRequest(int32_t requestId) = 0;

        virtual shared_ptr<HttpRequest> CreateHttpRequest(const std::string& url,
            int connect_timeout = support::HTTP_CONNECT_TIMEOUT,
            int receive_timeout = support::HTTP_RECEIVE_TIMEOUT,
            int request_timeout = support::HTTP_REQUEST_TIMEOUT,
            bool enable_logging = true,
            support::HttpRequestSecurityLevel security_level = support::HTTP_REQUEST_SECURITY_LEVEL_LOW) = 0;
    };

    typedef std::shared_ptr<IHttpClient> HttpClientPtr;
}  // namespace huestream

#endif  // HUESTREAM_COMMON_HTTP_IHTTPCLIENT_H_
