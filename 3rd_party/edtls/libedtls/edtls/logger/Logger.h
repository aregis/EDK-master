/*******************************************************************************
Copyright © 2016 Philips Lighting Holding B.V.
All Rights Reserved.
********************************************************************************/

#ifndef EDTLS_LOGGER_LOGGER_H_
#define EDTLS_LOGGER_LOGGER_H_

#include <edtls/wrapper/helpers.h>

#include <functional>
#include <string>

typedef void (*LogFunction)(const char *text);

class Logger {
 private:
  LogFunction logfunc_;
  std::string prefix_;

  std::string appendPrefixIfSet(const std::string &logline) {
    std::string prefixString = "";
    if (prefix_.length() > 0) {
      prefixString = "[" + prefix_ + "] ";
    }

    return prefixString + logline + "\n";
  }

 public:
  Logger(LogFunction logFunction, const char* prefix) : logfunc_(logFunction), prefix_(prefix) { }
  explicit Logger(LogFunction logFunction) : Logger(logFunction, "") { }
  Logger() : Logger(nullptr) { }

  void setLogFunction(LogFunction logfunc) {
    logfunc_ = logfunc;
  }

  LogFunction GetLogFunction() {
    return logfunc_;
  }

  void Log(const std::string logline) {
    if (!logfunc_) return;
    auto appendedLog = appendPrefixIfSet(logline);
    logfunc_(appendedLog.c_str());
  }

  template<typename ... Args>
  void Log(const std::string &format, Args ... args) {
    if (!logfunc_) return;
    auto text = string_format(format, args ...);
    text = appendPrefixIfSet(text);
    logfunc_(text.c_str());
  }
};

#endif  // EDTLS_LOGGER_LOGGER_H_
