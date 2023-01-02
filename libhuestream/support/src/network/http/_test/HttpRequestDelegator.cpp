/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include "support/network/http/_test/HttpRequestDelegator.h"

#include <string>
#include <mutex>
#include <cassert>

using std::string;
using std::mutex;
using std::unique_lock;

namespace support {

    /* provider */
    
    static shared_ptr<HttpRequestDelegateProvider> _delegate_provider = shared_ptr<HttpRequestDelegateProvider>(new HttpRequestDelegateProviderImpl());
    static mutex _delegate_provider_mutex;

    shared_ptr<HttpRequestDelegateProvider> HttpRequestDelegator::set_delegate_provider(shared_ptr<HttpRequestDelegateProvider> delegate_provider) {
        unique_lock<mutex> _delegate_provider_lock(_delegate_provider_mutex);
        _delegate_provider.swap(delegate_provider);
        return delegate_provider;
    }

    shared_ptr<HttpRequestDelegateProvider> HttpRequestDelegator::get_delegate_provider() {
        unique_lock<mutex> _delegate_provider_lock(_delegate_provider_mutex);
        return _delegate_provider;
    }



    /* delegator */

    HttpRequestDelegator::HttpRequestDelegator(const string& url, int connect_timeout, int receive_timeout, int request_timeout, bool enable_logging, HttpRequestSecurityLevel security_level) 
        : HttpRequest(url, connect_timeout, receive_timeout, request_timeout, enable_logging, security_level) {
        unique_lock<mutex> _delegate_provider_lock(_delegate_provider_mutex);
        _delegate = _delegate_provider->get_delegate(url, connect_timeout, receive_timeout, request_timeout, enable_logging, security_level);
        assert(_delegate != nullptr);
    }
                                                                    
    int HttpRequestDelegator::get_connect_timeout() {
        return _delegate->get_connect_timeout();
    }

    void HttpRequestDelegator::set_connect_timeout(int connect_timeout) {
        _delegate->set_connect_timeout(connect_timeout);
    }
    
    int HttpRequestDelegator::get_receive_timeout() {
        return _delegate->get_receive_timeout();
    }
    
    void HttpRequestDelegator::set_receive_timeout(int receive_timeout) {
        _delegate->set_receive_timeout(receive_timeout);
    }
    
    int HttpRequestDelegator::get_request_timeout() {
        return _delegate->get_request_timeout();
    }
    
    void HttpRequestDelegator::set_request_timeout(int request_timeout) {
        _delegate->set_request_timeout(request_timeout);
    }
    
    const string& HttpRequestDelegator::get_proxy_address() {
        return _delegate->get_proxy_address();
    }

    int HttpRequestDelegator::get_proxy_port() {
        return _delegate->get_proxy_port();
    }
    
    void HttpRequestDelegator::set_proxy(const string& proxy_address, int proxy_port) {
        _delegate->set_proxy(proxy_address, proxy_port);
    }
    
    string HttpRequestDelegator::get_content_type() {
        return _delegate->get_content_type();
    }

    void HttpRequestDelegator::set_content_type(const string& content_type) {
        _delegate->set_content_type(content_type);
    }

    void HttpRequestDelegator::add_header_field(const string& name, const string& value) {
        _delegate->add_header_field(name, value);
    }

    string HttpRequestDelegator::get_header_field_value(const string &field_name) const {
        return _delegate->get_header_field_value(field_name);
    }
    
    int HttpRequestDelegator::do_get(HttpRequestCallback callback) {
        return _delegate->do_get(callback);
    }
    
    int HttpRequestDelegator::do_post(const string& body, support::File* file, HttpRequestCallback callback) {
        return _delegate->do_post(body, file, callback);
    }

    int HttpRequestDelegator::do_put(const string& body, HttpRequestCallback callback) {
        return _delegate->do_put(body, callback);
    }

    int HttpRequestDelegator::do_delete(HttpRequestCallback callback) {
        return _delegate->do_delete(callback);
    }

    int HttpRequestDelegator::do_request(const string& method, const string& body, File* file, HttpRequestCallback callback) {
        return _delegate->do_request(method, body, file, callback);
    }

    void HttpRequestDelegator::set_trusted_certs(const std::vector<std::string>& trusted_certs) {
        _delegate->set_trusted_certs(trusted_certs);
    }

    void HttpRequestDelegator::expect_common_name(const std::string& common_name) {
        _delegate->expect_common_name(common_name);
    }

    void HttpRequestDelegator::set_verify_ssl(bool verify_ssl) {
        _delegate->set_verify_ssl(verify_ssl);
    }

    void HttpRequestDelegator::cancel() {
        _delegate->cancel();

        if (_executor != nullptr) {
            _executor->request_canceled(this);
        }
    }
    
    void HttpRequestDelegator::set_executor(support::HttpRequestExecutor *executor) {
        _delegate->set_executor(executor);
        _executor = executor;
    }

}  // namespace support
