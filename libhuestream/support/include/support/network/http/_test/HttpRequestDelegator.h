/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <memory>
#include <string>
#include <vector>

#include "support/network/http/HttpRequest.h"
#include "support/network/http/HttpRequestError.h"
#include "support/network/http/IHttpResponse.h"
#include "support/util/StringUtil.h"

using std::shared_ptr;
using std::string;

namespace support {

    class HttpRequestDelegateProvider {
    public:
        /**
         Get request delegate
         @param url The url to initialize the request
         @return The request delegate
         */
        virtual shared_ptr<HttpRequest> get_delegate(const string& url, int connect_timeout, int receive_timeout, int request_timeout, bool enable_logging = true, HttpRequestSecurityLevel security_level = HTTP_REQUEST_SECURITY_LEVEL_LOW) = 0;

        virtual ~HttpRequestDelegateProvider() = default;
    };
    
    // Default
    class HttpRequestDelegateProviderImpl : public HttpRequestDelegateProvider {
    public:
        /**
         @see HttpRequestDelegateProvider
         */
        shared_ptr<HttpRequest> get_delegate(const string& url, int connect_timeout, int receive_timeout, int request_timeout, bool enable_logging = true, HttpRequestSecurityLevel security_level = HTTP_REQUEST_SECURITY_LEVEL_LOW) {
            // Create instance of the real http_request implementation
            return shared_ptr<HttpRequest>(new HttpRequest(url, connect_timeout, receive_timeout, request_timeout, enable_logging, security_level));
        }
    };

    class HttpRequestDelegator : public HttpRequest {
    public:
        static const char* CONTENT_TYPE_APPLICATION_JSON;
        static const char* CONTENT_TYPE_APPLICATION_X_WWW_FORM_URLENCODED;

        /**
         @see HttpRequest.h
         */
        HttpRequestDelegator(const string& url,
                             int connect_timeout = HTTP_CONNECT_TIMEOUT,
                             int receive_timeout = HTTP_RECEIVE_TIMEOUT,
                             int request_timeout = HTTP_REQUEST_TIMEOUT,
                             bool enable_logging = true,
                             HttpRequestSecurityLevel security_level = HTTP_REQUEST_SECURITY_LEVEL_LOW);
        
        /**
         @see HttpRequest.h
         */
        int get_connect_timeout() override;
        
        /**
         @see HttpRequest.h
         */
        void set_connect_timeout(int connect_timeout) override;
        
        /**
         @see HttpRequest.h
         */
        int get_receive_timeout() override;
        
        /**
         @see HttpRequest.h
         */
        void set_receive_timeout(int receive_timeout) override;
        
        /**
         @see HttpRequest.h
         */
        int get_request_timeout() override;
        
        /**
         @see HttpRequest.h
         */
        void set_request_timeout(int request_timeout) override;
        
        /**
         @see HttpRequest.h
         */
        const string& get_proxy_address() override;

        /**
         @see HttpRequest.h
         */
        int get_proxy_port() override;

        /**
         @see HttpRequest.h
         */
        void set_proxy(const string& proxy_address, int proxy_port) override;
        
        /**
         @see HttpRequest.h
         */
        string get_content_type() override;
        
        /**
         @see HttpRequest.h
         */
        void set_content_type(const string& content_type) override;
        
        /**
         @see HttpRequest.h
         */
        void add_header_field(const string& name, const string& value) override;

        /**
         @see HttpRequest.h
         */
        string get_header_field_value(const string &field_name) const override;

        /**
         @see HttpRequest.h
         */
        int do_get(HttpRequestCallback callback) override;
        
        /**
         @see HttpRequest.h
         */
        int do_post(const string& body, File* file, HttpRequestCallback callback) override;
        
        /**
         @see HttpRequest.h
         */
        int do_put(const string& body, HttpRequestCallback callback) override;
        
        /**
         @see HttpRequest.h
         */
        int do_delete(HttpRequestCallback callback) override;
        
        /**
         @see HttpRequest.h
         */
        int do_request(const string& method, const string& body, File* file, HttpRequestCallback callback) override;
        
        /**
         @see HttpRequest.h
         */
        void cancel() override;

        /**
         @see HttpRequest.h
         */
        void set_executor(support::HttpRequestExecutor* executor) override;

        void set_trusted_certs(const std::vector<std::string>& trusted_certs) override;

        void expect_common_name(const std::string& common_name) override;

        void set_verify_ssl(bool) override;

        /* delegate provider */
        
        /**
         Set the delegate
         @note   Initially HttpRequestDelegateProviderImpl is set as delegate
         @return The previous delegate, nullptr if no delegate has been set
         */
        static shared_ptr<HttpRequestDelegateProvider> set_delegate_provider(shared_ptr<HttpRequestDelegateProvider> delegate_provider);

        static shared_ptr<HttpRequestDelegateProvider> get_delegate_provider();
        
    private:
        /** the request delegate */
        shared_ptr<HttpRequest> _delegate;
    };

}  // namespace support
