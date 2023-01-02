/*******************************************************************************
Copyright © 2016 Philips Lighting Holding B.V.
All Rights Reserved.
********************************************************************************/

#ifndef EDTLS_WRAPPER_ISERVERWRAPPER_H_
#define EDTLS_WRAPPER_ISERVERWRAPPER_H_

#include <edtls/server/ISessionHandler.h>
#include <edtls/wrapper/IPSKProvider.h>
#include <edtls/logger/Logger.h>

struct SocketBase { virtual ~SocketBase() { }};
struct SessionContextBase { virtual ~SessionContextBase() { }};


class IServerWrapper {
 protected:
  virtual ~IServerWrapper() = default;

 public:
  virtual bool init() = 0;
  virtual void deinit() = 0;
  virtual SocketBase* createSocket() = 0;
  virtual void freeSocket(SocketBase* socket) = 0;
  virtual void freeSession(SessionContextBase* session) = 0;
  virtual bool bind(SocketBase* socket, const char* address, const char *port) = 0;
  virtual SessionContextBase* createSession(const ISessionHandler &sessionHandler, unsigned int readTimeoutMs) = 0;
  virtual bool closeSession(SessionContextBase* session) = 0;
  virtual void resetSession(SessionContextBase* session) = 0;
  virtual bool listen(SocketBase* socket, SessionContextBase* session, unsigned int timeoutMs) = 0;
  virtual bool handshake(SessionContextBase* session) = 0;
  virtual unsigned int receive(SessionContextBase* session, char* buffer, unsigned int max_bytes) = 0;
};

#endif  // EDTLS_WRAPPER_ISERVERWRAPPER_H_
