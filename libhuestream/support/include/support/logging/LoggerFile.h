/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <mutex>
#include <string>
#include <fstream>  // NOLINT

#include "support/logging/Logger.h"

using std::mutex;
using std::ofstream;
using std::string;

namespace support {
    namespace log {
        
        class LoggerFile : public Logger {
        public:
            /**
             * Constructor
             */
            LoggerFile(LogLevel level, unsigned int enabled_components, const char* storage_location = "");
            
            /**
             Log the message to the console
             @param msg The message to be logged to the console
             */
            virtual void log(const string& timestamp, LogLevel loglevel, LogComponent component, const string& logtag, const string& msg);
            
        private:
            /** path to the folder where the logfile should be stored */
            std::string _storage_location;
            /** the file stream */
            ofstream _logstream;
        };
        
    }  // namespace log
}  // namespace support

