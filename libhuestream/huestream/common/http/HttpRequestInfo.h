/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#ifndef HUESTREAM_COMMON_HTTP_HTTPREQUESTINFO_H_
#define HUESTREAM_COMMON_HTTP_HTTPREQUESTINFO_H_

#include "huestream/common/serialize/SerializerHelper.h"

#include <string>
#include <memory>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <unordered_map>

#include "support/network/http/HttpRequestBase.h"
#include "huestream/common/data/Bridge.h"

namespace huestream {

#define HTTP_REQUEST_PUT "PUT"
#define HTTP_REQUEST_POST "POST"
#define HTTP_REQUEST_GET "GET"
#define HTTP_REQUEST_DELETE "DELETE"
#define COMMA ,

class HttpRequestInfo;
typedef std::shared_ptr<HttpRequestInfo> HttpRequestPtr;
typedef std::function<void()> HttpRequestInfoCallback;


class HttpRequestInfo {
 public:
    HttpRequestInfo();

    HttpRequestInfo(std::string method, std::string url, std::string body = "");

    virtual ~HttpRequestInfo(){}// = default;

    virtual void FinishRequest();

    virtual void WaitUntilReady();

    virtual bool IsReady();

    virtual bool StartRequest();

 PROP_DEFINE(HttpRequestInfo, std::string, url, Url);
 PROP_DEFINE(HttpRequestInfo, std::string, method, Method);
 PROP_DEFINE(HttpRequestInfo, std::string, body, Body);
 PROP_DEFINE(HttpRequestInfo, bool, success, Success);
 PROP_DEFINE(HttpRequestInfo, std::string, response, Response);
 PROP_DEFINE(HttpRequestInfo, uint32_t, statusCode, StatusCode)
 PROP_DEFINE(HttpRequestInfo, std::string, token, Token);
 PROP_DEFINE(HttpRequestInfo, HttpRequestInfoCallback, callback, Callback);
 PROP_DEFINE(HttpRequestInfo, uint32_t, roundTripTime, RoundTripTime)
 PROP_DEFINE(HttpRequestInfo, int, connectTimeout, ConnectTimeout);
 PROP_DEFINE(HttpRequestInfo, int, receiveTimeout, ReceiveTimeout);
 PROP_DEFINE(HttpRequestInfo, int, requestTimeout, RequestTimeout);
 PROP_DEFINE_BOOL(HttpRequestInfo, bool, enableLogging, LoggingEnabled);
 PROP_DEFINE(HttpRequestInfo, support::HttpRequestSecurityLevel, securityLevel, SecurityLevel);
 PROP_DEFINE_BOOL(HttpRequestInfo, bool, enableSslVerification, SslVerificationEnabled);
 PROP_DEFINE(HttpRequestInfo, std::string, expectedCommonName, ExpectedCommonName);
 PROP_DEFINE(HttpRequestInfo, std::vector<std::string>, trustedCertificates, TrustedCertificates);
 PROP_DEFINE(HttpRequestInfo, std::unordered_map<std::string COMMA std::string>, header, Header);
 PROP_DEFINE(HttpRequestInfo, std::string, fileName, FileName);
 PROP_DEFINE_BOOL(HttpRequestInfo, bool, enableMd5DigestGeneration, Md5DigestGenerationEnabled);

 protected:
    std::mutex _mutex;
    std::condition_variable _condition;
    bool _isReady;
    bool _isRunning;
};

}  // namespace huestream

#endif  // HUESTREAM_COMMON_HTTP_HTTPREQUESTINFO_H_
