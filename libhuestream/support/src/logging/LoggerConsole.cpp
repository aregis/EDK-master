/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#ifdef ANDROID_LOGCAT
#include <android/log.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#endif

#ifdef LOG_WITH_NSLOG
#import <Foundation/Foundation.h>
#endif

#include <iostream>  // NOLINT
#include <sstream>
#include <string>

#include "support/logging/LoggerConsole.h"
#include "support/AttributeSpecifiers.h"

using std::cout;
using std::endl;
using std::stringstream;
using std::unique_lock;

namespace support {
    namespace log {
        const unsigned int MAX_CHARACTERS_PER_LINE MAYBE_USED = 510;

        LoggerConsole::LoggerConsole(LogLevel level, unsigned int enabled_components)
        : Logger(level, enabled_components) {
        }

        void LoggerConsole::log(const string& timestamp, LogLevel loglevel, LogComponent component, const string& logtag, const string& msg) {
            (void)timestamp;
            (void)component;
#ifdef ANDROID_LOGCAT
            // Log to the android logcat output
            switch (loglevel) {
                case LEVEL_ERROR:
                    android_log(ANDROID_LOG_ERROR, logtag.c_str(), msg.c_str());
                    break;

                case LEVEL_WARN:
                    android_log(ANDROID_LOG_VERBOSE, logtag.c_str(), msg.c_str());
                    break;

                case LEVEL_INFO:
                    android_log(ANDROID_LOG_INFO, logtag.c_str(), msg.c_str());
                    break;

                case LEVEL_DEBUG:
                    android_log(ANDROID_LOG_DEBUG, logtag.c_str(), msg.c_str());
                    break;

                default:
                    break;
            }
#else
            // Log the message to the console

            stringstream msg_decorated;

            // Decorate the message
#ifndef LOG_WITH_NSLOG
            // timestamp already included in nslog
            msg_decorated << timestamp;
            msg_decorated << " ";
#endif
            msg_decorated << logtag;
            msg_decorated << "::";
            msg_decorated << get_component_str(component);
            msg_decorated << " ~ ";
            msg_decorated << get_level_str(loglevel);
            msg_decorated << ": ";

#ifdef LOG_WITH_NSLOG
            msg_decorated << msg;
            string str = msg_decorated.str();
            NSString* result = [NSString stringWithUTF8String:str.c_str()];
            NSLog(@"%@", result);
#else
            string prelude = msg_decorated.str();

            size_t max_msg_size = MAX_CHARACTERS_PER_LINE - prelude.size();

            std::istringstream input(msg);
            std::string line;

            // also split on newlines
            while (std::getline(input, line)) {
                // CLion console blocks with long lines, so we need to break it up in several lines
                for (size_t i = 0; i < line.size(); i += max_msg_size) {
                    cout << prelude << line.substr(i, max_msg_size) << endl;
                }
            }
#endif
#endif
        }

#ifdef ANDROID_LOGCAT
        void LoggerConsole::android_log(int prio, const char* tag, const char* message) {
            // if message exceeds 1023 characters, chop it up and print in loop
            size_t shift = 0;
            while (shift < strlen(message)) {
                __android_log_print(prio, tag, "%s", message+shift);
                shift += 1023;
            }
        }
#endif

    }  // namespace log
}  // namespace support
