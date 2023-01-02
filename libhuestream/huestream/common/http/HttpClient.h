/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#ifndef HUESTREAM_COMMON_HTTP_HTTPCLIENT_H_
#define HUESTREAM_COMMON_HTTP_HTTPCLIENT_H_

#include "huestream/common/http/IHttpClient.h"

#include <memory>
#include <string>

namespace huestream {

    class HttpClient : public IHttpClient {
    public:
        ~HttpClient() override;
        void Execute(HttpRequestPtr request) override;
        int32_t ExecuteAsync(HttpRequestPtr request, HttpRequestCallback callback = {}) override;
				void CancelAsyncRequest(int32_t requestId) override;
        shared_ptr<HttpRequest> CreateHttpRequest(const std::string& url,
            int connect_timeout = support::HTTP_CONNECT_TIMEOUT,
            int receive_timeout = support::HTTP_RECEIVE_TIMEOUT,
            int request_timeout = support::HTTP_REQUEST_TIMEOUT,
            bool enable_logging = true,
            support::HttpRequestSecurityLevel security_level = support::HTTP_REQUEST_SECURITY_LEVEL_LOW) override;

    private:
        struct Data {
            std::mutex _active_requests_mutex;
            bool _is_shutdown = false;
            std::vector<std::tuple<int32_t, HttpRequest*>> _active_requests;
        };
        std::shared_ptr<Data> _data = std::make_shared<Data>();
    };

}  // namespace huestream

#endif  // HUESTREAM_COMMON_HTTP_HTTPCLIENT_H_
