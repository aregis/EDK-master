/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <functional>
#include <memory>
#include <string>
#include <atomic>
#include <unordered_map>
#include <vector>

#include "support/network/http/HttpRequestConst.h"
#include "support/network/http/HttpRequestError.h"
#include "support/network/http/IHttpResponse.h"
#include "support/network/http/IHttpRequest.h"
#include "support/network/http/HttpRequestExecutor.h"
#include "support/util/File.h"
#include "support/util/StringUtil.h"
#include "support/logging/Log.h"

namespace support {
    /**
     * security level of the request
     * e.g.
     */
    enum HttpRequestSecurityLevel {
        HTTP_REQUEST_SECURITY_LEVEL_LOW,
        HTTP_REQUEST_SECURITY_LEVEL_HIGH
    };
    enum HttpRequestState {
        HTTP_REQUEST_STATE_INITIAL,
        HTTP_REQUEST_STATE_EXECUTING,
        HTTP_REQUEST_STATE_CANCELLED
    };

    typedef std::function<void (const support::HttpRequestError& error, const IHttpResponse& response)> HttpRequestCallback;

    typedef std::function<void (int64_t download_total, int64_t downloaded_upto_now,
                                int64_t uplodad_total, int64_t uploaded_upto_now)> HttpRequestProgressCallback;

    class HttpRequestBase : public IHttpRequest {
    public:
        /**
         set the executor of this request
         */
        virtual void set_executor(HttpRequestExecutor* executor);

        /**
         Get connect timeout in seconds
         @return The connect timeout in seconds
         */
        virtual int get_connect_timeout();

        /**
         Set connection timeout in seconds
         @param connection_timeout The connection timeout in seconds
         */
        virtual void set_connect_timeout(int connect_timeout);

        /**
         Get receive timeout in seconds
         @note Timeout when no data received for the given time
         @return The receive timeout in seconds
         */
        virtual int get_receive_timeout();

        /**
         Set read timeout in seconds
         @note Timeout when no data received for the given time
         @param request_timeout The receive timeout in seconds
         */
        virtual void set_receive_timeout(int receive_timeout);

        /**
         Get request timeout in seconds
         @return The request timeout in seconds
         */
        virtual int get_request_timeout();

        /**
         Set request timeout in seconds
         @param request_timeout The request timeout in seconds
         */
        virtual void set_request_timeout(int request_timeout);

        /**
         Get proxy address
         @return The proxy address
         */
        virtual const string& get_proxy_address();

        /**
         Get proxy port
         @return the proxy port
         */
        virtual int get_proxy_port();

        /**
         Set the proxy to use for this request
         @param proxy_address proxy address
         @param proxy_port proxy port
         */
        virtual void set_proxy(const string& proxy_address, int proxy_port);

        /**
         Get the content type
         @return The content type
         */
        virtual string get_content_type();

        /**
         Set the type of the content (body) that is sent
         @param content_type The content type
         */
        virtual void set_content_type(const string& content_type);

        /**
         add a header field
         @param field_name name of the field to set
         @param field_value value of the field to set
         */
        virtual void add_header_field(const string& field_name, const string& field_value);

        /**
         Get a header field value
         */
        string get_header_field_value(const string &field_name) const override;

        /**
         Set bearer authorization header
         */
        virtual void set_bearer_auth_header(const string& token);

        /**
         Set digest authentication header
         */
        virtual void set_digest_auth_header(const string& username,
                                            const string& realm,
                                            const string& nonce,
                                            const string& uri,
                                            const string& response);
        /**
         Do a GET request on the url
         @see do_request(...)
         */
        virtual int do_get(HttpRequestCallback callback);

        /**
         Do a POST request on the url
         @see do_request(...)
         */
        virtual int do_post(const string& body, File* file, HttpRequestCallback callback);

        /**
         Do a PUT request on the url
         @param the
         @see do_request(...)
         */
        virtual int do_put(const string& body, HttpRequestCallback callback);

        /**
         Do a POST request on the url
         @see do_request(...)
         */
        virtual int do_delete(HttpRequestCallback callback);

        /**
         Execute the request on a url
         @param method   The method to be executed on the url:
         - GET, POST, PUT, DELETE
         @param body     The body to send
         @param File     The file to upload
         @param callback The callback which will be called when connecting is done
         and provides a response object and an error object with a message
         and error code, which represents one of the following values:
         - HTTP_REQUEST_STATUS_OK:       request successful
         - HTTP_REQUEST_STATUS_FAILED:   request failed
         e.g. due to connection timeout
         - HTTP_REQUEST_STATUS_CANCELED: request cancelled
         The response object contains a http status code and the body
         @return The result code, which represents one of the following values:
         - HTTP_REQUEST_STATUS_OK:     request being executed
         - HTTP_REQUEST_STATUS_FAILED: could not execute request, because it's
         already busy executing
         */
        virtual int do_request(const string& method, const string& body, File* file, HttpRequestCallback callback);

        /**
         get the url
         @return server url
         */
        string get_url() const override;

        /**
         set the url
         @param url url
         */
        void set_url(const string& url) override;

        /**
         get the method to be executed on the url
         @return method (GET, POST, PUT, DELETE)
         */
        string get_method() const override;

        /**
         get the body to be sent
         @return body content
         */
        string get_body() const override;

        /**
         Set trusted certificates (root CA or exact match).
         If certificate chain is not trusted - error code in HttpRequestCallback is set to HTTP_REQUEST_ERROR_CODE_INVALID_CERTIFICATE.
         */
        virtual void set_trusted_certs(const std::vector<std::string>& trusted_certs);

        /**
         Set expected common name.
         */
        virtual void expect_common_name(const std::string& common_name);

        /**
         Set whether to verify peer via SSL protocol.
         */
        virtual void set_verify_ssl(bool);

        /**
         set true if you want to treat this request as external and to not influence bridge connection state
         */
        void set_external(bool is_external) override;

        /**
         returns true if request is for external use (doesnt influence bridge connection state)
         */
        bool is_external() const override;


        /*
         sets a callback for the download / upload bytewise progress
         */
        virtual void set_progress_callback(HttpRequestProgressCallback progress_callback);

        /*
         Set the file name for which a get request data will be written to.
        */
        virtual void set_file_name_to_write(const std::string& file_name);

        /*
         Set whether to compute the md5 digest of the body.
        */
        virtual void set_generate_md5_digest(bool);

    protected:
        /**
         Constructor
         */
        HttpRequestBase(const string& url,
                        int connect_timeout                     = HTTP_CONNECT_TIMEOUT,
                        int receive_timeout                     = HTTP_RECEIVE_TIMEOUT,
                        int request_timeout                     = HTTP_REQUEST_TIMEOUT,
                        bool enable_logging                     = true,
                        HttpRequestSecurityLevel security_level = HTTP_REQUEST_SECURITY_LEVEL_LOW);

        /**
         Notify all listeners that response has been received.
         @param error error information about performed actions.
         @param response received response.
         */
        void notify_monitor_response_received(const HttpRequestError& error, const IHttpResponse& response);

        /**
         @see do_request(...)
         */
        virtual int do_request_internal(const string& method, const string& body, File* file, HttpRequestCallback callback) = 0;

        /** the url to connect to */
        string                   _url;
        /** the method which is going to be used for execution of current request. This member is valid just in the moment of execution do_perform method  */
        mutable const string*    _method = nullptr;
        /** the body which is going to be send. This member is valid just in the moment of execution do_perform method  */
        mutable const string*    _body = nullptr;
        /** the connection timeout */
        int                      _connect_timeout;
        /** the receive timeout */
        int                      _receive_timeout;
        /** the request timeout */
        int                      _request_timeout;
        /** enable logging */
        bool                     _enable_logging;
        /** the proxy address */
        string                   _proxy_address;
        /** the proxy port */
        int                      _proxy_port;
        /** the executor of this request */
        HttpRequestExecutor*     _executor;
        /** logcomponent for this request */
        log::LogComponentType    _log_component;
        /** security level of this request; used to restrict monitoring */
        HttpRequestSecurityLevel _security_level;
        /** http request headers */
        std::unordered_map<string, string> _headers;

        std::string              _common_name;
        std::vector<std::string> _trusted_certs;
        bool                     _verify_ssl;
        bool                     _is_external;
        HttpRequestProgressCallback _progress_callback;
        std::string              _file_name_to_write;
        bool                     _generate_md5_digest;
    };
}  // namespace support

