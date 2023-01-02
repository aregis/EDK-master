/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/
#pragma once

#include <memory>
#include <string>

#include "support/logging/Log.h"
#include "support/logging/LoggerFile.h"

using std::string;
using std::shared_ptr;

namespace support {
    namespace log {

        class LoggerFileDelegateProvider {
        public:
            /**
             Get request delegate
             @return The request delegate
             */
            virtual shared_ptr<LoggerFile> get_delegate() = 0;

            virtual ~LoggerFileDelegateProvider() = default;
        };

        // Default
        class LoggerFileDelegateProviderImpl : public LoggerFileDelegateProvider {
        public:
            LoggerFileDelegateProviderImpl(LogLevel level, unsigned int enabled_components, const char* storage_location = "") {
                _delegate = shared_ptr<LoggerFile>(new LoggerFile(level, enabled_components, storage_location));
            }
            /**
             @see LoggerFileDelegateProvider
             */
            shared_ptr<LoggerFile> get_delegate() {
                // Create instance of the real http_request implementation
                return _delegate;
            }
        private:
            shared_ptr<LoggerFile> _delegate;
        };

        class LoggerFileDelegate {
        public:
            /**
             @see logging/LoggerFile.h
             */
            virtual void log(const string& timestamp, LogLevel loglevel, LogComponent component, const string& logtag, const string& msg) = 0;
        };

        class LoggerFileDelegator : public LoggerFile {
        public:
            /**
             @see logging/LoggerFile.h
             */
            LoggerFileDelegator(LogLevel level, unsigned int enabled_components, const char *storage_location = "");

            /**
             @see logging/LoggerFile.h
             */
            void log(const string &timestamp, LogLevel loglevel, LogComponent component, const string &logtag,
                     const string &msg);

            /**
             Set the delegate
             @note   Initially LoggerFileDelegateProviderImpl is set as delegate
             @return The delegate, nullptr if no delegate has been set
            */
            static void set_delegate_provider(shared_ptr<LoggerFileDelegateProvider> delegate_provider);

            /**
             Get default delegate
             @return The default delegate
            */
            static shared_ptr<LoggerFile> get_default_delegate();

        private:
            static std::mutex _delegate_provider_mutex;
            static std::shared_ptr<LoggerFileDelegateProvider> _delegate_provider;
            static std::shared_ptr<LoggerFileDelegateProvider> _default_delegate_provider;

            static std::shared_ptr<std::mutex> get_delegate_provider_mutex();
        };

    }  // namespace log
}  // namespace support
