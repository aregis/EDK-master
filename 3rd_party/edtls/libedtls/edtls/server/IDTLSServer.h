/*******************************************************************************
Copyright © 2016 Philips Lighting Holding B.V.
All Rights Reserved.
********************************************************************************/

#ifndef EDTLS_SERVER_IDTLSSERVER_H_
#define EDTLS_SERVER_IDTLSSERVER_H_

#include <edtls/server/ISessionHandler.h>

class IDTLSServer: public ISessionHandler {
 public:
  virtual ~IDTLSServer() = default;
  virtual void start_async(const char *address, const char *port, unsigned int readTimeoutSeconds) = 0;
  virtual void stop() = 0;
};

#endif  // EDTLS_SERVER_IDTLSSERVER_H_
