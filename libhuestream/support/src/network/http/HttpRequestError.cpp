/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <string>

#include "support/network/http/HttpRequestError.h"

using std::string;

namespace support {

    HttpRequestError::HttpRequestError(const string& message, ErrorCode code) : _message(string(message)),
                                                                                _code(code) { }
    
    const string& HttpRequestError::get_message() const {
        return _message;
    }
    
    void HttpRequestError::set_message(const string& message) {
        _message = string(message);
    }
    
    HttpRequestError::ErrorCode HttpRequestError::get_code() const {
        return _code;
    }
    
    void HttpRequestError::set_code(ErrorCode code) {
        _code = code;
    }
    
}  // namespace support
