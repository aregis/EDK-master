/*******************************************************************************
Copyright © 2016 Philips Lighting Holding B.V.
All Rights Reserved.
********************************************************************************/

#ifndef EDTLS_WRAPPER_ICLIENTWRAPPER_H_
#define EDTLS_WRAPPER_ICLIENTWRAPPER_H_

#include <edtls/wrapper/IClientConnectionHandler.h>
#include <edtls/wrapper/PSKInfo.h>
#include <edtls/logger/Logger.h>

class IClientWrapper {
 protected:
  Logger logger_;
  virtual bool seed_random_number_generator() = 0;
  virtual bool config_PSK(const PSKInfo &pskInfo) = 0;
  explicit IClientWrapper(Logger logger) : logger_(logger) { }
  virtual ~IClientWrapper() { }

 public:
  virtual bool init(IClientConnectionHandler *connectionHandler, unsigned int maxConnectTimeoutMs, unsigned int minConnectTimeoutMs) = 0;
  virtual void deinit() = 0;
  virtual void close() = 0;
  virtual bool connect(const char *address, const char *port, const PSKInfo &pskInfo) = 0;
  virtual bool handshake() = 0;
  virtual unsigned int send(char *buffer, unsigned int size_bytes) = 0;
};

#endif  // EDTLS_WRAPPER_ICLIENTWRAPPER_H_
