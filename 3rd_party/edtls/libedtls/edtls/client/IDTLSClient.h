/*******************************************************************************
Copyright © 2016 Philips Lighting Holding B.V.
All Rights Reserved.
********************************************************************************/

#ifndef EDTLS_CLIENT_IDTLSCLIENT_H_
#define EDTLS_CLIENT_IDTLSCLIENT_H_

#include <edtls/wrapper/PSKInfo.h>
#include <edtls/wrapper/IClientConnectionHandler.h>

class IDTLSClient: public IClientConnectionHandler {
 public:
  virtual bool connect(const char *address, const char *port, const PSKInfo &pskInfo) = 0;
  virtual void close() = 0;
  virtual unsigned int send(char *buffer, unsigned int size_bytes) = 0;
};

#endif  // EDTLS_CLIENT_IDTLSCLIENT_H_
