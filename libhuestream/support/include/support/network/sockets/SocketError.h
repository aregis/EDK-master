/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <string>

#include "support/util/StringUtil.h"

using std::string;

namespace support {

    class SocketError {
    public:
        /**
         Construct without data
         */
        SocketError() = default;
 
        /**
         Construct with data
         @param message The error message
         @param code    The error code
         */
        SocketError(const string& message, int code);
        
        /**
         Get message
         @return The error message
         */
        const string& get_message() const;
        
        /** 
         Set message
         @param message The error message
         */
        void set_message(const string& message);
        
        /**
         Get code
         @param The error code
         */
        int get_code() const;
        
        /**
         Set code
         @param code The error code
         */
        void set_code(int code);
        
    private:
        /** error message */
        string _message;
        /** error code */
        int    _code;
    };
    
}  // namespace support

