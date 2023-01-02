/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include "support/network/http/HttpRequestTask.h"

using support::HttpRequestTask;

#include "support/network/http/_test/HttpRequestDelegator.h"
#define HTTP_REQUEST HttpRequestDelegator

struct HttpRequestTask::HttpRequest : public support::HTTP_REQUEST {
    using support::HTTP_REQUEST::HTTP_REQUEST;
};

HttpRequestTask::HttpRequestTask(const std::string& url, const Options* options /* = nullptr */)
    : _request{new HttpRequest{url}} {
    if (options) {
        _request->set_connect_timeout(options->connect_timeout);
        _request->set_receive_timeout(options->receive_timeout);
        _request->set_request_timeout(options->request_timeout);
        if (options->use_proxy) {
            _request->set_proxy(options->proxy_address, options->proxy_port);
        }
        _request->set_verify_ssl(options->verify_ssl);
    }
}

void HttpRequestTask::execute(CompletionHandler done) {
    _request->do_get([this, done](const HttpRequestError& error, const IHttpResponse& response) {
        _response.reset(response.clone().release());
        _error.reset(new HttpRequestError{error});
        done();
    });
}

void HttpRequestTask::stop() {
    _request->cancel();
}

IHttpResponse* HttpRequestTask::get_response() const {
    return _response.get();
}

HttpRequestError* HttpRequestTask::get_error() const {
    return _error.get();
}

void HttpRequestTask::set_connect_timeout(int connect_timeout) {
    _request->set_connect_timeout(connect_timeout);
}

void HttpRequestTask::set_receive_timeout(int receive_timeout) {
    _request->set_receive_timeout(receive_timeout);
}

void HttpRequestTask::set_request_timeout(int request_timeout) {
    _request->set_request_timeout(request_timeout);
}

void HttpRequestTask::set_proxy(const string &proxy_address, int proxy_port) {
    _request->set_proxy(proxy_address, proxy_port);
}

HttpRequestTask::~HttpRequestTask() = default;
