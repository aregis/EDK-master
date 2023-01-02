/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <mutex>
#include <string>

#include "support/logging/Logger.h"

using std::string;
using std::mutex;

namespace support {
    namespace log {
        
        class LoggerConsole : public Logger {
        public:
            /**
             * Constructor
             */
            LoggerConsole(LogLevel level, unsigned int enabled_components);

            /**
             Log the message to the console
             @param msg The message to be logged to the console
             */
            virtual void log(const string& timestamp, LogLevel loglevel, LogComponent component, const string& logtag, const string& msg);
            
        private:
#ifdef ANDROID_LOGCAT
            void android_log(int prio, const char* tag, const char* message);
#endif
        };
    
    }  // namespace log
}  // namespace support

