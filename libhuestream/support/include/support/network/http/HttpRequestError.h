/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <string>

namespace support {

    class HttpRequestError {
    public:
        typedef enum ErrorCode {
            HTTP_REQUEST_ERROR_CODE_SUCCESS                     = 0, /* no error occurred */
            HTTP_REQUEST_ERROR_CODE_UNDEFINED                   = 1, /* unknown or not defined */
            HTTP_REQUEST_ERROR_CODE_CANCELED                    = 2, /* request was canceled */
            HTTP_REQUEST_ERROR_CODE_CANNOT_RESOLVE_HOST         = 3, /* could not resolve host */
            HTTP_REQUEST_ERROR_CODE_CANNOT_CONNECT              = 4, /* can not connect */
            HTTP_REQUEST_ERROR_CODE_TIMEOUT                     = 5, /* connection timeout */
            HTTP_REQUEST_ERROR_CODE_SERVER_RESET                = 6, /* got nothing */
            HTTP_REQUEST_ERROR_CODE_INVALID_CERTIFICATE         = 7, /* invalid certificate */
            HTTP_REQUEST_ERROR_CODE_GATEWAY_TIMEOUT             = 8, /* gateway timeout */
            HTTP_REQUEST_ERROR_CODE_INVALID_EVENTING_CONNECTION = 9, /* http2 eventing connection to the bridge became invalid */
        } ErrorCode;

        /**
         Construct without data
         */
        HttpRequestError() = default;

        /**
         Construct with data
         @param message The error message
         @param code    The error code
         */
        HttpRequestError(const std::string& message, ErrorCode code);

        /**
         Get message
         @return The error message
         */
        const std::string& get_message() const;

        /**
         Set message
         @param message The error message
         */
        void set_message(const std::string& message);

        /**
         Get code
         @return The error code
         */
        ErrorCode get_code() const;

        /**
         Set code
         @param code The error code
         */
        void set_code(ErrorCode code);

    protected:
        /** the error message */
        std::string _message;
        /** the error code */
        ErrorCode _code;
    };

}  // namespace support

