/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <string>
#include <memory>
#include <mutex>

#include "support/logging/_test/LoggerFileDelegator.h"

using std::unique_lock;
using std::mutex;

namespace support {
    namespace log {

        /* static members */
        shared_ptr<LoggerFileDelegateProvider> LoggerFileDelegator::_delegate_provider;
        shared_ptr<LoggerFileDelegateProvider> LoggerFileDelegator::_default_delegate_provider;
        std::mutex LoggerFileDelegator::_delegate_provider_mutex;

        void LoggerFileDelegator::set_delegate_provider(shared_ptr<LoggerFileDelegateProvider> delegate_provider) {
            unique_lock <mutex> delegate_provider_lock(_delegate_provider_mutex);

            _delegate_provider = delegate_provider;
        }

        /* delegator */
        LoggerFileDelegator::LoggerFileDelegator(LogLevel level, unsigned int enabled_components,
                                                 const char *storage_location) : LoggerFile(level, enabled_components,
                                                                                            storage_location) {
            _default_delegate_provider = shared_ptr<LoggerFileDelegateProvider>(new LoggerFileDelegateProviderImpl(level, enabled_components, storage_location));
        }

        void LoggerFileDelegator::log(const string &timestamp, LogLevel loglevel, LogComponent component,
                                      const string &logtag, const string &msg) {
            shared_ptr<LoggerFile> delegate;
            {
                unique_lock <mutex> _delegate_provider_lock(_delegate_provider_mutex);

                if (_delegate_provider != nullptr) {
                    delegate = _delegate_provider->get_delegate();
                } else {
                    delegate = _default_delegate_provider->get_delegate();
                }
            }
            if (delegate) {
                delegate->log(timestamp, loglevel, component, logtag, msg);
            }
        }

        shared_ptr<LoggerFile> LoggerFileDelegator::get_default_delegate() {
            return _default_delegate_provider->get_delegate();
        }

    }  // namespace log
}  // namespace support

