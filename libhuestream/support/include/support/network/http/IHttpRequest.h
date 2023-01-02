/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/
#pragma once

#include <string>

#include "support/network/http/HttpRequestError.h"
#include "support/network/http/IHttpResponse.h"

namespace support {

    enum HttpRequestReturnCode {
        HTTP_REQUEST_STATUS_OK,
        HTTP_REQUEST_STATUS_FAILED   = -1,
        HTTP_REQUEST_STATUS_CANCELED = -2,
    };

    class IHttpRequest {
    public:
        /**
         Cancel this request
         */
        virtual void cancel() = 0;
        
        /**
         Destructor
         */
        virtual ~IHttpRequest() = default;

        /**
         get a header field
         @param field_name name of the field
         @return value of the field
         */
        virtual std::string get_header_field_value(const std::string &field_name) const = 0;

        /**
         get the url
         @return server url
         */
        virtual std::string get_url() const = 0;

        /**
         set the url
         @param url url
         */
        virtual void set_url(const std::string& url) = 0;

        /**
         get the method to be executed on the url
         @return method (GET, POST, PUT, DELETE)
         */
        virtual std::string get_method() const = 0;

        /**
         get the body to be sent
         @return body content
         */
        virtual std::string get_body() const = 0;

        /**
         set true if you want to treat this request as external and to not influence bridge connection state
         */
        virtual void set_external(bool is_external) = 0;

        /**
         returns true if request is for external use (doesnt influence bridge connection state)
         */
        virtual bool is_external() const = 0;
    };
}  // namespace support

