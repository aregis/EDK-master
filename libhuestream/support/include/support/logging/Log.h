/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <mutex>
#include <map>
#include <memory>
#include <string>
#include <thread>

#include "support/logging/Logger.h"

#include "support/util/VectorOperations.h"

using std::map;
using std::unique_ptr;
using std::shared_ptr;
using std::string;
using std::thread;
using std::mutex;

#define HUE_LOG   support::log::log
#define HUE_DEBUG support::log::debug
#define HUE_INFO  support::log::info
#define HUE_ERROR support::log::error
#define HUE_WARN  support::log::warn
#define HUE_ENDL  support::log::endl

#define HUE_CORE            support::log::core
#define HUE_SUPPORT         support::log::support
#define HUE_WRAPPER         support::log::wrapper
#define HUE_CLIENT          support::log::client
#define HUE_NETWORK         support::log::network
#define HUE_COLORCONVERSION support::log::colorconversion
#define HUE_STREAM          support::log::stream
#define HUE_STREAM_DTLS     support::log::streamdtls
#define HUE_APPCORE         support::log::appcore
#define HUE_TEST            support::log::test
#define HUE_IGNORE          support::log::ignore

namespace support {
    namespace log {

        typedef enum LoggerType {
            LOGGER_TYPE_CONSOLE = 0,
            LOGGER_TYPE_FILE    = 1,
            LOGGER_TYPE_COUNT   = 2
        } LoggerType;

        typedef enum LogType {
            error,
            warn,
            info,
            debug
        } LogType;

        typedef enum LogComponentType {
            unknown,
            network,
            support,
            core,
            wrapper,
            client,
            colorconversion,
            stream,
            streamdtls,
            appcore,
            test,
            ignore,
        } LogComponentType;

        typedef std::function<void(const std::string&, LogLevel level)> LogCallback;

        /**
         The log entry is used by threads to compose their
         log message. Each thread id gets its own entry
         */
        struct LogEntry {
            /** */
            LogType type;
            /** */
            string  msg;
            /** */
            LogComponentType component;
        };

        class Log {
        public:
            /**
             Construct log, with the following default values:
             - level:   LEVEL_INFO
             - name:    "HueSDK"
             - loggers: console, file
             At startup only the console logger will be initialized, at INFO level
             */
            Log();

            /**
             Copying and moving not allowed
             */
            Log(Log&& log)                   = delete;
            Log& operator=(Log&& other)      = delete;
            Log(const Log& log)              = delete;
            Log& operator=(const Log& other) = delete;

            /**
             Set the log level and enabled components for the console logger
             @param level The log level
             @param enabled_components A bitmask of the enabled components
             */
            void set_console_log(LogLevel level = LEVEL_INFO, unsigned int enabled_components = COMPONENT_ALL);

            /**
             Set the log level and enabled components for the file logger
             @param level The log level
             @param enabled_components A bitmask of the enabled components
             */
            void set_file_log(LogLevel level = LEVEL_INFO, unsigned int enabled_components = COMPONENT_ALL);

            /**
             Set the location in which the file logs will be stored
             @param storage_location The storage location
             */
            void set_storage_location(const char* storage_location);

            void set_log_callback(LogCallback callback);

            /**
             Set log type for the thread entry
             @param log  The log
             @param type The type to set
             @return The log
             */
            friend Log& operator << (Log& log, LogType type);

            /**
             Set log component type for the thread entry
             @param log  The log
             @param component The component to set
             @return The log
             */
            friend Log& operator << (Log& log, LogComponentType component);

            /**
             Append message for the thread entry
             @param log The log
             @param msg The message to append
             @return The log
             */
            friend Log& operator << (Log& log, const char* msg);

            /**
             Append message for the thread entry
             @param log The log
             @param msg The message to append
             @return The log
             */
            friend Log& operator << (Log& log, const string& msg);

            /**
             Append integer value for the thread entry
             @param log  The log
             @param val The integer to append
             @return The log
             */
            friend Log& operator << (Log& log, int64_t val);

            /**
             Append pointer value for the thread entry
             @param log  The log
             @param val The integer to append
             @return The log
             */
            friend Log& operator << (Log& log, void* val);

            template<typename T>
            friend Log& operator<<(Log& log, T value) {
                return log << to_string(value);
            }

            /**
             @see endl()
             */
            friend Log& operator << (Log& log, Log& (*fp)());

            /**
             When done composing, the global var endl should be used for
             finishing the log entry and pass the message to the loggers
             This method will be called by globar var and takes care of
             passing the message to all the active loggers. This construction
             allows to use the endl like used for std::cout:

             HUE_LOG << HUE_SUPPORT <<  HUE_DEBUG << "this is a debug message" << HUE_ENDL;
             */
            Log& endl();

#ifdef ANDROID_LOGCAT
            /**
             Redirect stdout to hue log
             Useful for logging the output of gtest to logcat
             */
            static void redirect_stdout_to_log();
#endif

        private:
            /** the name of the log */
            string                                _name;
            /** array with all the active loggers */
            unique_ptr<Logger>                    _loggers[LOGGER_TYPE_COUNT];
            /** ensure thread safety when changing and accessing loggers */
            mutex                                 _loggers_mutex;
            /** container with all the entries for each thread id */
            map<thread::id, shared_ptr<LogEntry>>  _thread_entries;
            /** ensure thread safety in managing the container */
            mutex                                 _thread_entries_mutex;

            LogCallback                           _log_callback;

            /**
             Get entry by the current thread id
             If no entry exist, a new entry will be created
             @return The entry for the thread id
             */
            shared_ptr<LogEntry> get_thread_entry();

            /**
             Remove entry by the current thread id. This method will be called
             when endl() has finished passing the logging message to the loggers
             */
            void remove_thread_entry();

            /**
             Get log level by the log type
             @return The log level
             */
            static LogLevel get_level(LogType type);

            /**
             Get log component by the log component type
             @return The Log component
             */
            static LogComponent get_component_from_type(LogComponentType type);
        };

        /** global log, which has a similar implementation as std::cout */
        extern Log log;
        /** global endl, which has a similar implementation as std::endl */
        extern Log& endl();

    }  // namespace log
}  // namespace support

