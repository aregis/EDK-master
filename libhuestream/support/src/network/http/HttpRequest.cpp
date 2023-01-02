/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/


#include <regex>
#include <string>
#include <cassert>

#include "support/network/Network.h"
#include "support/network/NetworkConfiguration.h"
#include "support/network/http/HttpRequest.h"
#include "support/network/http/HttpRequestBase.h"
#include "support/network/http/HttpRequestParams.h"
#include "support/network/http/IHttpClient.h"
#include "support/network/http/IHttpResponse.h"
#include "support/util/UrlUtil.h"

#ifdef OBJC_HTTP_CLIENT
#   include "support/network/http/objc/ObjcHttpClient.h"
#else
#   include "support/network/http/curl/CurlHttpClient.h"
#endif

using std::regex;
using std::regex_search;
using std::smatch;
using std::string;

namespace support {

    static std::mutex                                   _instance_mutex;
    static std::shared_ptr<IHttpClient>                 _client_instance;


    std::shared_ptr<IHttpClient> HttpRequest::get_default_http_client() {
        std::lock_guard<std::mutex> lk(_instance_mutex);

        if (!_client_instance) {
            reset_http_client_internal();
        }

        return _client_instance;
    }

    void HttpRequest::set_default_http_client(const std::shared_ptr<IHttpClient>& client) {
        std::lock_guard<std::mutex> lk(_instance_mutex);
        _client_instance = client;
    }

    void HttpRequest::reset_http_client() {
        std::lock_guard<std::mutex> lk(_instance_mutex);
        reset_http_client_internal();
    }

    void HttpRequest::reset_http_client_internal() {
#ifdef OBJC_HTTP_CLIENT
        _client_instance.reset(new ObjcHttpClient);
#else
        _client_instance.reset(new CurlHttpClient);
#endif
    }

    std::shared_ptr<IHttpClient> HttpRequest::get_http_client() {
        return _http_client;
    }

    /* httprequest implementation */

    HttpRequest::HttpRequest(const std::string& url, int connect_timeout, int receive_timeout, int request_timeout, bool enable_logging, HttpRequestSecurityLevel security_level) :
            HttpRequestBase(url, connect_timeout, receive_timeout, request_timeout, enable_logging, security_level), _handle(),
            _http_client(get_default_http_client()) {
    }

    HttpRequest::~HttpRequest() {
        cancel();
    }

    HttpRequestParams HttpRequest::get_request_params(const std::string& method, const std::string& body, File* file) const {
        HttpRequestParams data;

        data.url = _url;
        data.proxy_address = _proxy_address;
        data.proxy_port = _proxy_port;
        data.connect_timeout = _connect_timeout;
        data.receive_timeout = _receive_timeout;
        data.request_timeout = _request_timeout;
        data.enable_logging = _enable_logging;
        data.log_component = _log_component;
        data.security_level = _security_level;
        data.headers = _headers;
        data.method = method;
        data.body = body;
        data.file = file;
        data.common_name = _common_name;
        data.trusted_certs = _trusted_certs;
        data.verify_ssl = _verify_ssl;
        data.progress_callback = _progress_callback;
        data.file_name = _file_name_to_write;
        data.generate_md5_digest = _generate_md5_digest;

        if (data.trusted_certs.empty()) {
            data.trusted_certs = NetworkConfiguration::get_trusted_certificates(data.url);
        }

        if (NetworkConfiguration::is_ssl_check_disabled()) {
            data.verify_ssl = false;
        }

        return data;
    }

    int HttpRequest::do_request_internal(const std::string& method, const std::string& body, File* file, HttpRequestCallback original_callback) {
        HUE_LOG << _log_component <<  HUE_DEBUG << "HttpRequest: url: " << _url << ", method: " << method << ", body: " << body << HUE_ENDL;

        if (file != nullptr) {
            set_content_type(HTTP_CONTENT_TYPE_MULTIPART_FORM_DATA);
        }

        HttpRequestParams data{get_request_params(method, body, file)};

        auto callback = [this, original_callback](const support::HttpRequestError& error, const IHttpResponse& response) {
            HUE_LOG << _log_component <<  HUE_DEBUG << "HttpRequest: " << error.get_message() << ", status: " << response.get_status_code() << ", body: " << response.get_body() << HUE_ENDL;

            notify_monitor_response_received(error, response);
            original_callback(error, response);
        };

        {
            std::lock_guard<std::mutex> lk(_mutex);

            if (_handle) {
                get_http_client()->stop_request(_handle);
            }

            data.interface_name = get_private_network_interface_name_for_url(_url);
            _handle = get_http_client()->start_request(std::move(data), callback);
        }

        return HTTP_REQUEST_STATUS_OK;
    }

    string HttpRequest::get_private_network_interface_name_for_url(string url) {
        string name;
#       ifdef  ANDROID
        const NetworkInterface network_interface(support::UrlUtil::get_host(url), NetworkInetType::INET_IPV4);
        if (network_interface.is_private()) {
            name = Network::get_local_interface_name(NetworkInterface::ip_to_uint(network_interface.get_ip()));
        }
#       else
        (void)url;
#       endif  // ANDROID
        return name;
    }

    void HttpRequest::cancel() {
        {
            std::lock_guard<std::mutex> lk(_mutex);

            if (_handle) {
                get_http_client()->stop_request(_handle);
                _handle = {};
            }
        }

        if (_executor != nullptr) {
            _executor->request_canceled(this);
        }
    }
}  // namespace support
