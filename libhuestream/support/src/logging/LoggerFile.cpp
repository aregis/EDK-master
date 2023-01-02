/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <iostream>  // NOLINT(readability/streams)
#include <string>

#include "support/logging/LoggerFile.h"

using std::string;

namespace support {
    namespace log {
        LoggerFile::LoggerFile(LogLevel level, unsigned int enabled_components, const char* storage_location)
                : Logger(level, enabled_components),
                  _storage_location(storage_location) {}

        void LoggerFile::log(const string& timestamp, LogLevel loglevel, LogComponent component, const string& logtag, const string& msg) {
            if (!_logstream.is_open()) {
                if (_storage_location.empty() || _storage_location == ":memory:") {
                    // the user did not define a path in persistence yet
                    // try again next time
                    return;
                }

                _logstream.open(_storage_location + "/" + get_time_str(true) + ".log");

                if (_logstream.fail()) {
                    _logstream.clear();
                    return;
                }
            }

            _logstream << timestamp;
            _logstream << " ";
            _logstream << logtag;
            _logstream << "::";
            _logstream << get_component_str(component);
            _logstream << " ~ ";
            _logstream << get_level_str(loglevel);
            _logstream << ": ";
            _logstream << msg;
            _logstream << std::endl;
        }

    }  // namespace log
}  // namespace support
