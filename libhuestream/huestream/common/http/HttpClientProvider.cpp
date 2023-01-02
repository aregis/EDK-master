/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <huestream/common/http/HttpClientProvider.h>
#include <huestream/common/http/IHttpClient.h>
#include <huestream/common/http/HttpClient.h>

#include <memory>

namespace huestream {

    HttpClientPtr HttpClientProvider::_httpClient = std::static_pointer_cast<IHttpClient>(std::make_shared<HttpClient>());

    HttpClientPtr HttpClientProvider::GetHttpClient() {
        return _httpClient;
    }

    void HttpClientProvider::SetCustomHttpClient(HttpClientPtr httpClient) {
        _httpClient = httpClient;
    }

}  // namespace huestream
