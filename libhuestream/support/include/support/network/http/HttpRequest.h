/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <memory>
#include <string>

#include "support/network/http/HttpRequestBase.h"
#include "support/network/http/IHttpClient.h"
#include "support/util/File.h"
#include "support/util/StringUtil.h"

namespace support {

    class HttpRequestExecutor;

    class HttpRequest : public HttpRequestBase {
    public:
        /**
         * Set the http client that will be used for FUTURE requests
         */
        static void set_default_http_client(const std::shared_ptr<IHttpClient> &client);

        static void reset_http_client();

        /**
         Constructor
         */
        explicit HttpRequest(const std::string &url,
                             int connect_timeout                     = HTTP_CONNECT_TIMEOUT,
                             int receive_timeout                     = HTTP_RECEIVE_TIMEOUT,
                             int request_timeout                     = HTTP_REQUEST_TIMEOUT,
                             bool enable_logging                     = true,
                             HttpRequestSecurityLevel security_level = HTTP_REQUEST_SECURITY_LEVEL_LOW);

        /**
         Destructor, which will block until the http request is cancelled completely
         */
        virtual ~HttpRequest();

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
        int do_request_internal(const std::string& method, const std::string& body, File* file, HttpRequestCallback callback) override;

        /**
         Cancel the request. This method will block until the request is completely cancelled.
         It's also guaranteed, if still busy executing the request, that the callback will be called
         with the HTTP_REQUEST_STATUS_CANCELED error code
         */
        void cancel() override;

    protected:
        std::mutex          _mutex;
        IHttpClient::Handle _handle;

        HttpRequestParams get_request_params(const std::string& method, const std::string& body, File* file) const;

        /**
         * Returns the name of the network interface related to the URL
         * Note: Will only work on Android. Will return empty string on other targets.
         * @return Name of the network interface related to the URL or empty string
         */
        virtual string get_private_network_interface_name_for_url(std::string url);

    private:
        std::shared_ptr<IHttpClient> _http_client;

        static std::shared_ptr<IHttpClient> get_default_http_client();

        static void reset_http_client_internal();

        std::shared_ptr<IHttpClient> get_http_client();
    };
}  // namespace support
