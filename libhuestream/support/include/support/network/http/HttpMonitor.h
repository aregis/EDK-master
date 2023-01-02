/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include "support/network/http/HttpMonitorObserver.h"

namespace support {

    class HttpMonitor {
    public:
        /**
         Register observer that wants to receive http events.
         @param observer pointer to the observer object.
         */
        static void add_observer(HttpMonitorObserver* observer);

        /**
         Unregister observer in order to not receive http events anymore.
         @param observer pointer to the observer object.
         */
        static void remove_observer(HttpMonitorObserver* observer);

        /**
         Notify all listeners that request has been performed.
         @param request request that was performed.
         @param status  status of performed action.
         */
        static void notify_request_performed(const IHttpRequest& request, int status);

        /**
         Notify all listeners that response has been received.
         @param error error information about performed actions.
         @param response received response.
         */
        static void notify_response_received(const HttpRequestError& error, const IHttpResponse& response);
    };

}  // namespace support

