/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#ifndef HUESTREAM_COMMON_HTTP_IBRIDGEHTTPCLIENT_H_
#define HUESTREAM_COMMON_HTTP_IBRIDGEHTTPCLIENT_H_

#include <string>
#include <memory>

#include "huestream/common/http/IHttpClient.h"
#include "huestream/common/http/HttpRequestInfo.h"

namespace huestream {

    class IBridgeHttpClient {
    public:
        virtual ~IBridgeHttpClient() = default;
        virtual HttpRequestPtr ExecuteHttpRequest(BridgePtr bridge, const std::string& method, const std::string& url, const std::string& body = {}, bool supportEvent = false) = 0;
        virtual int32_t ExecuteHttpRequest(BridgePtr bridge, const std::string& method, const std::string& url, const std::string& body, HttpRequestCallback callback, bool supportEvent = false) = 0;
        virtual void CancelHttpRequest(int32_t requestId) = 0;
    };

    typedef std::shared_ptr<IBridgeHttpClient> BridgeHttpClientPtr;
}  // namespace huestream

#endif  // HUESTREAM_COMMON_HTTP_IBRIDGEHTTPCLIENT_H_
