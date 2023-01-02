/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include "support/network/http/IHttpResponse.h"
#include "support/network/http/IHttpRequest.h"
#include "support/network/http/HttpRequestError.h"

namespace support {

    class HttpMonitorObserver {
    public:
        /**
         Method called when a request has been performed.
         @param request request that has been performed.
         @param status  status of performed action. Check HttpRequestReturnCode for expected values.
         */
        virtual void on_request_performed(const IHttpRequest& request, int status) = 0;

        /**
         Method called when a response has been received.
         @param resopnse response that has been received.
         @param error    error information about received response.
         */
        virtual void on_response_received(const IHttpResponse& response, const HttpRequestError& error) = 0;

        virtual ~HttpMonitorObserver() = default;
    };

}  // namespace support

