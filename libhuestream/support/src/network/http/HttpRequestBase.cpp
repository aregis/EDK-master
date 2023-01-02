/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include "support/network/http/HttpRequestBase.h"

#include <string.h>
#include <string>

#include "support/network/http/HttpMonitor.h"

#ifdef _WIN32
#define snprintf _snprintf
#endif

namespace support {

    HttpRequestBase::HttpRequestBase(const string& url, int connect_timeout, int receive_timeout, int request_timeout, bool enable_logging, HttpRequestSecurityLevel security_level) :
        _url(url),
        _connect_timeout(connect_timeout),
        _receive_timeout(receive_timeout),
        _request_timeout(request_timeout),
        _enable_logging(enable_logging),
        _proxy_address(""),
        _proxy_port(0),
        _executor(nullptr),
        _log_component(HUE_NETWORK),
        _security_level(security_level),
        _verify_ssl(true),
        _is_external(false),
        _generate_md5_digest(false) {
        if (!enable_logging) {
            _log_component = HUE_IGNORE;
        }
    }

    int HttpRequestBase::get_connect_timeout() {
        return _connect_timeout;
    }

    void HttpRequestBase::set_connect_timeout(int connect_timeout) {
        _connect_timeout = connect_timeout;
    }

    int HttpRequestBase::get_receive_timeout() {
        return _receive_timeout;
    }

    void HttpRequestBase::set_receive_timeout(int receive_timeout) {
        _receive_timeout = receive_timeout;
    }

    int HttpRequestBase::get_request_timeout() {
        return _request_timeout;
    }

    void HttpRequestBase::set_request_timeout(int request_timeout) {
        _request_timeout = request_timeout;
    }

    void HttpRequestBase::set_executor(HttpRequestExecutor* executor) {
        _executor = executor;
    }

    const string& HttpRequestBase::get_proxy_address() {
        return _proxy_address;
    }

    int HttpRequestBase::get_proxy_port() {
        return _proxy_port;
    }

    void HttpRequestBase::set_proxy(const string& proxy_address, int proxy_port) {
        _proxy_address = proxy_address;
        _proxy_port    = proxy_port;
    }

    string HttpRequestBase::get_content_type() {
        return get_header_field_value(HTTP_HEADER_CONTENT_TYPE);
    }

    void HttpRequestBase::set_content_type(const string& content_type) {
        add_header_field(HTTP_HEADER_CONTENT_TYPE, content_type);
    }

    void HttpRequestBase::set_bearer_auth_header(const string& token) {
        string header = HTTP_HEADER_BEARER_AUTHENTICATION + token;

        add_header_field(HTTP_HEADER_AUTHORIZATION, header);
    }

    void HttpRequestBase::set_digest_auth_header(const string& username,
                                                 const string& realm,
                                                 const string& nonce,
                                                 const string& uri,
                                                 const string& response) {
        size_t size = strlen(HTTP_HEADER_AUTHORIZATION_DIGEST_FORMAT)
            + username.size() + realm.size()
            + nonce.size()    + uri.size()
            + response.size();

        string authorization_header;
        // Resize string
        authorization_header.resize(size);

        snprintf(&authorization_header[0], size, HTTP_HEADER_AUTHORIZATION_DIGEST_FORMAT, username.c_str(), realm.c_str(), nonce.c_str(), uri.c_str(), response.c_str());
        authorization_header.shrink_to_fit();
        // Add header
        add_header_field(HTTP_HEADER_AUTHORIZATION, authorization_header);
    }

    void HttpRequestBase::add_header_field(const string& name, const string& value) {
        _headers[name] = value;
    }

    string HttpRequestBase::get_header_field_value(const string &key) const {
        auto it = _headers.find(key);
        if (it != _headers.end()) {
            return it->second;
        } else {
            return string();
        }
    }

    int HttpRequestBase::do_get(HttpRequestCallback callback) {
        string method = "GET";
        string body   = "";

        // Do a get request on the url
        return do_request(method, body, nullptr, callback);
    }

    int HttpRequestBase::do_post(const string& body, File* file, HttpRequestCallback callback) {
        string method = "POST";

        // Do a post request on the url
        return do_request(method, body, file, callback);
    }

    int HttpRequestBase::do_put(const string& body, HttpRequestCallback callback) {
        string method = "PUT";

        // Do a put request on the url
        return do_request(method, body, nullptr, callback);
    }

    int HttpRequestBase::do_delete(HttpRequestCallback callback) {
        string method = "DELETE";
        string body   = "";

        // Do a delete request on the url
        return do_request(method, body, nullptr, callback);
    }

    int HttpRequestBase::do_request(const string& method, const string& body, File* file, HttpRequestCallback callback) {
        auto result = do_request_internal(method, body, file, callback);

        _method = &method;
        _body = &body;

        HttpMonitor::notify_request_performed(*this, result);

        _method = nullptr;
        _body = nullptr;

        return result;
    }

    string HttpRequestBase::get_url() const {
        return _url;
    }

    void HttpRequestBase::set_url(const string& url) {
        _url = url;
    }

    string HttpRequestBase::get_method() const {
        return _method ? *_method : "";
    }

    string HttpRequestBase::get_body() const {
        return _body ? *_body : "";
    }

    void HttpRequestBase::notify_monitor_response_received(const HttpRequestError& error, const IHttpResponse& response) {
#       ifndef DEBUG
        if (_security_level != HTTP_REQUEST_SECURITY_LEVEL_HIGH) {
#       endif

        HttpMonitor::notify_response_received(error, response);
#       ifndef DEBUG
        }
#       endif
    }

    void HttpRequestBase::set_trusted_certs(const std::vector<string>& trusted_certs) {
        _trusted_certs = trusted_certs;
    }

    void HttpRequestBase::expect_common_name(const std::string& common_name) {
        _common_name = common_name;
    }

    void HttpRequestBase::set_verify_ssl(bool value) {
        _verify_ssl = value;
    }

    void HttpRequestBase::set_external(bool is_external) {
        _is_external = is_external;
    }

    bool HttpRequestBase::is_external() const {
        return _is_external;
    }

    void HttpRequestBase::set_progress_callback(HttpRequestProgressCallback progress_callback) {
        _progress_callback = std::move(progress_callback);
    }

    void HttpRequestBase::set_file_name_to_write(const std::string& file_name) {
        _file_name_to_write = file_name;
    }

    void HttpRequestBase::set_generate_md5_digest(bool value) {
        _generate_md5_digest = value;
    }
}  // namespace support
