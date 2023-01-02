/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <memory>
#include <string>
#include <vector>

#include "support/network/http/IHttpResponse.h"

namespace support_unittests {

    class MockHttpResponse : public support::IHttpResponse {
    public:
        MOCK_CONST_METHOD0(get_status_code, unsigned int());
        MOCK_CONST_METHOD0(get_body, std::string());
        MOCK_CONST_METHOD0(get_body_size, std::size_t());
        MOCK_CONST_METHOD1(get_header_field_value, const char*(const char*));
        MOCK_CONST_METHOD0(get_certificate_chain, std::vector<std::string>());
        MOCK_CONST_METHOD0(clone, std::unique_ptr<IHttpResponse>());
    };

}  // namespace support_unittests