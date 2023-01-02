/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <string>

#if defined(CONSOLE_SUPPORT)
#else
#include "support/date/Date.h"
#include "support/date/DateFormatter.h"
#endif

#include "support/logging/Logger.h"

using std::string;

namespace support {
    namespace log {

        Logger::Logger(LogLevel level, unsigned int enabled_components)
        : _level(level), _enabled_components(enabled_components) {
        }
        
        Logger::~Logger() {
        }

        bool Logger::is_enabled_for(LogLevel level, LogComponent component) const {
            return (level <= _level) && (_enabled_components & component);
        }

        void Logger::set_level(LogLevel level) {
            _level = level;
        }

        LogLevel Logger::get_level() const {
            return _level;
        }

        void Logger::set_enabled_components(unsigned int enabled_components) {
            _enabled_components = enabled_components;
        }

        unsigned int Logger::get_enabled_components() const {
            return _enabled_components;
        }

        string Logger::get_level_str(LogLevel level) {
            string level_str;
            
            switch (level) {
                case LEVEL_ERROR:
                    level_str = "<ERROR>  ";
                    break;
                    
                case LEVEL_WARN:
                    level_str = "<WARNING>";
                    break;
                    
                case LEVEL_INFO:
                    level_str = "<INFO>   ";
                    break;
                    
                case LEVEL_DEBUG:
                    level_str = "<DEBUG>  ";
                    break;
                    
                default:
                    break;
            }
            
            return level_str;
        }

        string Logger::get_component_str(LogComponent component) {
            string component_str;
            
            switch (component) {
                case COMPONENT_CORE:
                    component_str = "Core       ";
                    break;
                case COMPONENT_NETWORK:
                    component_str = "Network    ";
                    break;
                case COMPONENT_SUPPORT:
                    component_str = "Support    ";
                    break;
                case COMPONENT_WRAPPER:
                    component_str = "Wrapper    ";
                    break;
                case COMPONENT_CLIENT:
                    component_str = "Client     ";
                    break;
                case COMPONENT_COLORCONVERSION:
                    component_str = "Color      ";
                    break;
                case COMPONENT_STREAM:
                    component_str = "Stream     ";
                    break;
                case COMPONENT_STREAM_DTLS:
                    component_str = "Stream DTLS";
                    break;
                case COMPONENT_APPCORE:
                    component_str = "AppCore    ";
                    break;
                case COMPONENT_TEST:
                    component_str = "Test       ";
                    break;
                case COMPONENT_UNKNOWN:
                default:
                    component_str = "Unknown    ";
                    break;
            }
            
            return component_str;
        }

        string Logger::get_time_str(bool filename_safe) {
#if defined(_ORBIS) || defined(_NS) || defined(_DURANGO)
            string disableTimestamp = "TimeStr disabled";
            return disableTimestamp;
#else
            std::stringstream time_str;
            
            Date date;
            // Create date formatter
            DateFormatter date_formatter(filename_safe ? "%Y-%m-%dT%H-%M-%S." : "%Y-%m-%d %H:%M:%S.");
            // Convert date to string
            time_str << date_formatter.to_string(date, Timezone::local());
            
            time_str.fill('0');
            time_str.width(3);
            // Get milliseconds
            time_str << date.get_milliseconds();
            
            return time_str.str();
#endif
        }
    }  // namespace log
}  // namespace support
