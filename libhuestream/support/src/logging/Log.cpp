/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include "support/logging/Log.h"

#ifdef ANDROID_LOGCAT
#include <unistd.h>
#endif

#include <iomanip>
#include <map>
#include <string>

#include "support/logging/LoggerConsole.h"

#include "support/logging/_test/LoggerFileDelegator.h"
#define LOGGER_FILE LoggerFileDelegator

using std::stringstream;
using std::thread;
using std::unique_lock;

namespace support {
    namespace log {

        const std::string LOG_NULLPTR = "(nullptr)";

        Log::Log() : _name("HueSDK") {
            _loggers[LOGGER_TYPE_CONSOLE] = unique_ptr<Logger>(new LoggerConsole(LEVEL_INFO, COMPONENT_ALL));
            _loggers[LOGGER_TYPE_FILE]    = unique_ptr<Logger>(new LOGGER_FILE(LEVEL_OFF, COMPONENT_ALL, ""));
        }

        void Log::set_console_log(LogLevel level, unsigned int enabled_components) {
            unique_lock<mutex> lock(_loggers_mutex);

            _loggers[LOGGER_TYPE_CONSOLE]->set_level(level);
            _loggers[LOGGER_TYPE_CONSOLE]->set_enabled_components(enabled_components);
        }

        void Log::set_file_log(LogLevel level, unsigned int enabled_components) {
            unique_lock<mutex> lock(_loggers_mutex);

            _loggers[LOGGER_TYPE_FILE]->set_level(level);
            _loggers[LOGGER_TYPE_FILE]->set_enabled_components(enabled_components);
        }

        void Log::set_storage_location(const char* storage_location) {
            unique_lock<mutex> lock(_loggers_mutex);

            auto level = _loggers[LOGGER_TYPE_FILE]->get_level();
            auto enabled_components = _loggers[LOGGER_TYPE_FILE]->get_enabled_components();

            _loggers[LOGGER_TYPE_FILE] = unique_ptr<Logger>(new LOGGER_FILE(level, enabled_components, storage_location));
        }

        void Log::set_log_callback(LogCallback callback) {
            _log_callback = callback;
        }

        Log& operator << (Log& log_instance, LogType type) {
            shared_ptr<LogEntry> entry = log_instance.get_thread_entry();
            entry->type = type;

            return log;
        }

        Log& operator << (Log& log_instance, LogComponentType component) {
            shared_ptr<LogEntry> entry = log_instance.get_thread_entry();
            entry->component = component;

            return log;
        }

        Log& operator << (Log &log_instance, const char* msg) {
            shared_ptr<LogEntry> entry = log_instance.get_thread_entry();
            if (msg != nullptr) {
                entry->msg += to_string(msg);
            } else {
                entry->msg += LOG_NULLPTR;
            }

            return log;
        }

        Log& operator << (Log &log_instance, const string& msg) {
            shared_ptr<LogEntry> entry = log_instance.get_thread_entry();
            entry->msg += msg;

            return log;
        }

        Log& operator << (Log& log_instance, int64_t val) {
            shared_ptr<LogEntry> entry = log_instance.get_thread_entry();
            entry->msg += to_string(val);

            return log;
        }

        Log& operator << (Log& log_instance, void* val) {
            shared_ptr<LogEntry> entry = log_instance.get_thread_entry();

            if (val != nullptr) {
                entry->msg += to_string(val);
            } else {
                entry->msg += LOG_NULLPTR;
            }

            return log;
        }

        Log& operator << (Log& /*log*/, Log& (*fp)()) {
            return fp();
        }

        Log& Log::endl() {
            shared_ptr<LogEntry> entry = get_thread_entry();

            if (entry->component != ignore) {
                string timestamp = Logger::get_time_str();
                LogLevel level = get_level(entry->type);
                LogComponent component = get_component_from_type(entry->component);

                {
                    unique_lock<mutex> lock(_loggers_mutex);

                    if (_log_callback) {
                        _log_callback(entry->msg, level);
                    }

                    for (auto &iter : _loggers) {
                        if (iter->is_enabled_for(level, component)) {
                            iter->log(timestamp, level, component, _name, entry->msg);
                        }
                    }
                }
            }
            remove_thread_entry();

            return *this;
        }

        /* private */

        shared_ptr<LogEntry> Log::get_thread_entry() {
            unique_lock<mutex> lock(_thread_entries_mutex);

            // Get current thread id
            thread::id id = std::this_thread::get_id();

            map<thread::id, shared_ptr<LogEntry>>::iterator it = _thread_entries.find(id);
            // Check if an entry already exists for the thread id
            if (it == _thread_entries.end()) {
                // Create entry for the thread id
                shared_ptr<LogEntry> entry = shared_ptr<LogEntry>(new LogEntry());
                // Set default values
                entry->type = info;
                entry->msg  = "";
                entry->component = unknown;

                // Set the entry in the map
                _thread_entries[id] = entry;
            }

            return _thread_entries[id];
        }

        void Log::remove_thread_entry() {
            unique_lock<mutex> lock(_thread_entries_mutex);

            // Get current thread id
            thread::id id = std::this_thread::get_id();

            map<thread::id, shared_ptr<LogEntry>>::iterator it = _thread_entries.find(id);
            // Check if an entry exists for the thread id
            if (it != _thread_entries.end()) {
                // Remove the entry
                _thread_entries.erase(id);
            }
        }

        LogLevel Log::get_level(LogType type) {
            LogLevel level = LEVEL_OFF;

            switch (type) {
                case error:
                    level = LEVEL_ERROR;
                    break;

                case warn:
                    level = LEVEL_WARN;
                    break;

                case info:
                    level = LEVEL_INFO;
                    break;

                case debug:
                    level = LEVEL_DEBUG;
                    break;
            }

            return level;
        }

        LogComponent Log::get_component_from_type(LogComponentType type) {
            switch (type) {
                case core:
                    return COMPONENT_CORE;
                case network:
                    return COMPONENT_NETWORK;
                case support:
                    return COMPONENT_SUPPORT;
                case wrapper:
                    return COMPONENT_WRAPPER;
                case client:
                    return COMPONENT_CLIENT;
                case colorconversion:
                    return COMPONENT_COLORCONVERSION;
                case stream:
                    return COMPONENT_STREAM;
                case streamdtls:
                    return COMPONENT_STREAM_DTLS;
                case ignore:
                    return COMPONENT_NETWORK;
                case appcore:
                    return COMPONENT_APPCORE;
                case test:
                    return COMPONENT_TEST;
                default:
                    return COMPONENT_UNKNOWN;
            }
         }

#ifdef ANDROID_LOGCAT
        void Log::redirect_stdout_to_log() {
            std::thread t([]{
                /* make stdout line-buffered and stderr unbuffered */
                setvbuf(stdout, 0, _IOLBF, 0);
                setvbuf(stderr, 0, _IONBF, 0);

                /* create the pipe and redirect stdout and stderr */
                int pfd[2];
                pipe(pfd);
                dup2(pfd[1], 1);
                dup2(pfd[1], 2);

                ssize_t rdsz;
                char buf[1024];
                while ((rdsz = read(pfd[0], buf, sizeof(buf) - 1)) > 0) {
                    std::stringstream ss(std::string(buf, rdsz));
                    std::string line;

                    while (std::getline(ss, line, '\n')) {
                        HUE_LOG << HUE_DEBUG << HUE_CORE << line << HUE_ENDL;
                    }
                }
            });

            t.detach();
        }
#endif

        /** @see Log.h */
        Log log;
        /** @see Log.h */
        Log& endl() {
            return log.endl();
        }
    }  // namespace log
}  // namespace support
