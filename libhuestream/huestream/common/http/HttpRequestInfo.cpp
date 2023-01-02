/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <huestream/common/http/HttpRequestInfo.h>

#include <string>
#include <support/network/http/HttpRequestConst.h>

namespace huestream {

HttpRequestInfo::HttpRequestInfo() : HttpRequestInfo("", "", "") {
}

HttpRequestInfo::HttpRequestInfo(std::string method, std::string url, std::string body) :
    _url(url),
    _method(method),
    _body(body),
    _success(false),
    _response(""),
    _statusCode(0),
    _token(""),
    _roundTripTime(0),
    _isReady(false),
    _isRunning(false),
    _enableSslVerification(true),
    _connectTimeout(support::HTTP_CONNECT_TIMEOUT),
    _receiveTimeout(support::HTTP_RECEIVE_TIMEOUT),
    _requestTimeout(support::HTTP_REQUEST_TIMEOUT),
    _enableLogging(true),
    _securityLevel(support::HTTP_REQUEST_SECURITY_LEVEL_LOW),
    _fileName(""),
    _enableMd5DigestGeneration(false) {
}

PROP_IMPL(HttpRequestInfo, std::string, url, Url);
PROP_IMPL(HttpRequestInfo, std::string, method, Method);
PROP_IMPL(HttpRequestInfo, std::string, body, Body);
PROP_IMPL(HttpRequestInfo, bool, success, Success);
PROP_IMPL(HttpRequestInfo, std::string, response, Response)
PROP_IMPL(HttpRequestInfo, uint32_t, statusCode, StatusCode)
PROP_IMPL(HttpRequestInfo, std::string, token, Token);
PROP_IMPL(HttpRequestInfo, HttpRequestInfoCallback, callback, Callback);
PROP_IMPL(HttpRequestInfo, uint32_t, roundTripTime, RoundTripTime);
PROP_IMPL(HttpRequestInfo, int, connectTimeout, ConnectTimeout);
PROP_IMPL(HttpRequestInfo, int, receiveTimeout, ReceiveTimeout);
PROP_IMPL(HttpRequestInfo, int, requestTimeout, RequestTimeout);
PROP_IMPL_BOOL(HttpRequestInfo, bool, enableLogging, LoggingEnabled);
PROP_IMPL(HttpRequestInfo, support::HttpRequestSecurityLevel, securityLevel, SecurityLevel);
PROP_IMPL_BOOL(HttpRequestInfo, bool, enableSslVerification, SslVerificationEnabled);
PROP_IMPL(HttpRequestInfo, std::string, expectedCommonName, ExpectedCommonName);
PROP_IMPL(HttpRequestInfo, std::vector<std::string>, trustedCertificates, TrustedCertificates);
PROP_IMPL(HttpRequestInfo, std::unordered_map<std::string COMMA std::string>, header, Header);
PROP_IMPL(HttpRequestInfo, std::string, fileName, FileName);
PROP_IMPL_BOOL(HttpRequestInfo, bool, enableMd5DigestGeneration, Md5DigestGenerationEnabled);

void HttpRequestInfo::WaitUntilReady() {
    std::unique_lock<std::mutex> lock(_mutex);
    _condition.wait(lock, [this]() -> bool { return _isReady; });
}

bool HttpRequestInfo::IsReady() {
    std::unique_lock<std::mutex> lock(_mutex);
    return _isReady;
}

void HttpRequestInfo::FinishRequest() {
    std::unique_lock<std::mutex> lock(_mutex);
    _isReady = true;
    _isRunning = false;
    _condition.notify_all();
    lock.unlock();
    if (_callback != nullptr) {
        _callback();
    }
}

bool HttpRequestInfo::StartRequest() {
    std::unique_lock<std::mutex> lock(_mutex);
    if (_isRunning) {
        return false;
    } else {
        _isRunning = true;
        _isReady = false;
        return true;
    }
}

}  // namespace huestream
