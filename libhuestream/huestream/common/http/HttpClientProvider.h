/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#ifndef HUESTREAM_COMMON_HTTP_HTTPCLIENTPROVIDER_H_
#define HUESTREAM_COMMON_HTTP_HTTPCLIENTPROVIDER_H_

#include "huestream/common/http/IHttpClient.h"

namespace huestream {

    class HttpClientProvider {
    public:
        static HttpClientPtr GetHttpClient();

        /*
         * set a custom HttpClient to provide
         */
        static void SetCustomHttpClient(HttpClientPtr httpClient);

    private:
        static HttpClientPtr _httpClient;
    };

}  // namespace huestream

#endif  // HUESTREAM_COMMON_HTTP_HTTPCLIENTPROVIDER_H_
