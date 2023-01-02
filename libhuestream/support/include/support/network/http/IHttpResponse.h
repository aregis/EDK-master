/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <cstdlib>
#include <memory>
#include <vector>
#include <string>

#define HUESDK_LIB_NETWORK_HTTP_RESPONSE_STATUS_CODE_OK (200)
#define HUESDK_LIB_NETWORK_HTTP_RESPONSE_STATUS_CODE_NO_CONTENT (204)

namespace support {

    class IHttpResponse {
    public:
        /**
         destructor
         */
        virtual ~IHttpResponse() {}

        /**
         Get status code
         @return The http status code
         */
        virtual unsigned int get_status_code() const = 0;

        /**
         Get body
         @return The http body
         */
        virtual std::string get_body() const = 0;

        /**
         Get body size
         @return The http body size
         */
        virtual size_t get_body_size() const = 0;

        /**
         Get the http header field with the given field name
         @param field_name name of the requested field
         @return the http header field value of the field with the corresponding name, or an empty string if it was not found
         */
        virtual const char* get_header_field_value(const char* field_name) const = 0;

        /**
         Get the certificate chain for https connections
         */
        virtual std::vector<std::string> get_certificate_chain() const = 0;

        /**
         Get the md5 digest of the body
        */
        virtual std::string get_md5_digest() const = 0;

        /**
         Get the size of the downloaded file
        */
        virtual size_t get_file_size() const = 0;

        /**
         Get the local port number
        */
        virtual long get_local_port() const = 0;

        /**
         Clone the http response
         @return The cloned http response
         */
        virtual std::unique_ptr<IHttpResponse> clone() const = 0;
    };
}  // namespace support

