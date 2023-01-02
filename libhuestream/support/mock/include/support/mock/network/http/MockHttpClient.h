/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <gmock/gmock.h>

#include <string>

#include "support/network/http/IHttpClient.h"
#include "support/network/http/HttpRequestParams.h"
#include "support/network/http/HttpRequestBase.h"

namespace support_unittests {

    class MockHttpClient : public support::IHttpClient {
    public:
        MOCK_METHOD2(start_request, Handle(const support::HttpRequestParams&, support::HttpRequestCallback));

        MOCK_METHOD1(stop_request, void(Handle));
    };

}  // namespace support_unittests