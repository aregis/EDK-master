/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#ifndef HUESTREAM_COMMON_HTTP_BRIDGEHTTPCLIENT_H_
#define HUESTREAM_COMMON_HTTP_BRIDGEHTTPCLIENT_H_

#include "huestream/common/http/IBridgeHttpClient.h"

#include <memory>
#include <string>

namespace huestream {

    class BridgeHttpClient : public IBridgeHttpClient {
    public:
        BridgeHttpClient(HttpClientPtr httpClient);

        HttpRequestPtr ExecuteHttpRequest(BridgePtr bridge, const std::string& method, const std::string& url, const std::string& body = {}, bool supportEvent = false) override;
        int32_t ExecuteHttpRequest(BridgePtr bridge, const std::string& method, const std::string& url, const std::string& body, HttpRequestCallback callback, bool supportEvent = false) override;
				virtual void CancelHttpRequest(int32_t requestId) override;

    private:
        HttpRequestPtr CreateRequest(BridgePtr bridge, const std::string& method, const std::string& url, const std::string& body, bool supportEvent = false) const;
        HttpRequestCallback LinkHttpClientCallbackWithBridge(BridgePtr bridge, HttpRequestCallback original_callback);
        int32_t ExecuteRequestInternal(BridgePtr bridge, HttpRequestPtr request, HttpRequestCallback callback);

        HttpClientPtr _httpClient;
    };

}  // namespace huestream

#endif  // HUESTREAM_COMMON_HTTP_BRIDGEHTTPCLIENT_H_
