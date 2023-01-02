/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "support/util/StringUtil.h"
#include "support/network/http/IHttpResponse.h"

namespace support {

    class HttpResponse : public IHttpResponse {
    public:
        /**
         Construct without data
         */
        HttpResponse();

        /**
         Destructor
         */
        ~HttpResponse() = default;

        /**
         Construct with data
         @param status_code The http status code
         @param body        The http body
         */
        HttpResponse(unsigned int status_code, const char* body);

        /**
         Construct with data
         @param status_code The http status code
         @param body        The http body
         @param body_size   By default -1, which means the size of the body will be counteds
                            until the first null terminating character
         */
        HttpResponse(unsigned int status_code, const char* body, size_t body_size);

        /**
         @see support/network/http/IHttpResponse.h
         */
        unsigned int get_status_code() const;

        /**
         Set status code
         @param status_code The http status code
         */
        void set_status_code(unsigned int status_code);

        /**
         @see support/network/http/IHttpResponse.h
         */
        std::string get_body() const;

        /**
         Set body
         @param body The http body
         */
        void set_body(const char* body);

        /**
         Set body
         @param body The http body
         */
        void set_body(const std::string& body);

        /**
         @see support/network/http/IHttpResponse.h
         */
        size_t get_body_size() const;

        /**
         @see support/network/http/IHttpResponse.h
         */
        const char* get_header_field_value(const char* field_name) const;

        /**
         add a header field
         @param name name of the header field
         @param value value of the header field
         */
        void add_header_field(const char* name, const char* value);

        /**
         Set the certificate chain
         */
        void set_certificate_chain(const std::vector<std::string>& certificate_chain);

        /**
         @see support/network/http/IHttpResponse.h
         */
        std::vector<std::string> get_certificate_chain() const;

        /**
         Set md5 digest
         @param digest md5 digest of the body
        */
        void set_md5_digest(const std::string& digest);

        /**
         @see support/network/http/IHttpResponse.h
        */
        std::string get_md5_digest() const;

        /**
         Set the downloaded file size
         @param fileSize size of the file
        */
        void set_file_size(size_t fileSize);

        /**
         @see support/network/http/IHttpResponse.h
        */
        size_t get_file_size() const;

        /**
         Set the local port number
        */
        void set_local_port(long local_port);

        /**
         @see support/network/http/IHttpResponse.h
        */
        long get_local_port() const;

        /**
         @see support/network/http/IHttpResponse.h
         */
        virtual std::unique_ptr<IHttpResponse> clone() const;

    private:
        typedef std::map<std::string, std::string> HttpFieldMap;

        static const char* EMPTY_STRING;

        /** the http status code */
        unsigned int _status_code;
        /** the http body */
        std::string  _body;

        /** the http header */
        HttpFieldMap _header_fields;

        /** certificate chain */
        std::vector<std::string> _certificate_chain;

        std::string _md5_digest;

        size_t _file_size;

        long _local_port;
    };

}  // namespace support

