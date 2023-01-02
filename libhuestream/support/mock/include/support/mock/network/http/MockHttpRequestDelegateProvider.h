/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <gmock/gmock.h>

#include <memory>
#include <string>

#include "support/network/http/HttpRequest.h"
#include "support/network/http/_test/HttpRequestDelegator.h"

using std::shared_ptr;
using std::string;
using support::HttpRequest;
using support::HttpRequestDelegateProvider;
using support::HttpRequestSecurityLevel;

namespace support_unittests {

    class MockHttpRequestDelegateProvider : public HttpRequestDelegateProvider {
    public:
        /** 
        
         */
        MOCK_METHOD6(get_delegate, shared_ptr<HttpRequest>(const string& url, int connection_timeout, int receive_timeout, int request_timeout, bool enable_logging, HttpRequestSecurityLevel security_level));
    };

}  // namespace support_unittests

