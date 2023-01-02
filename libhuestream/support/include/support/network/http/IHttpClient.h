/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <string>

#include "support/network/http/HttpRequestParams.h"

namespace support {

    /*
        Notes on implementing an IHttpClient:

        - The HttpRequest class will call stop_request exactly once for every
          request returned from start_request. That is, it will either call
          stop_request when the request is being canceled, or when the httprequest
          object is being destroyed.

        - The callback will be allowed to be freed immediately after stop_request
          is called, therefore the http client must not call the callback after
          stop_request returns, and if the callback is currently being called,
          stop_request must wait for it to finish.

        - The http client must make sure that the callback will be called exactly
          once during the lifetime of the request. That is, it must either be called
          because the request finished by itself, or because stop_request was called
          before that.
    */

    class IHttpClient {
    public:
        using Handle = void*;

        virtual ~IHttpClient() = default;

        virtual Handle start_request(const HttpRequestParams& data, HttpRequestCallback callback) = 0;

        virtual void stop_request(Handle request) = 0;
    };

}  // namespace support

