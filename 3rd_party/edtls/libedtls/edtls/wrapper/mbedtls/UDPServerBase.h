/*******************************************************************************
Copyright © 2016 Philips Lighting Holding B.V.
All Rights Reserved.
********************************************************************************/

#ifndef EDTLS_WRAPPER_MBEDTLS_UDPSERVERBASE_H_
#define EDTLS_WRAPPER_MBEDTLS_UDPSERVERBASE_H_

#include <edtls/wrapper/mbedtls/UDPBase.h>

class UDPServerBase: public UDPBase {
 public:
  virtual netstatus_e Bind(const char *server_ip, unsigned int port) = 0;
  virtual netstatus_e Accept(ISocketContext* client_ctx, const char *client_ip_buf, size_t client_ip_buf_len, size_t *ip_len) = 0;
  void FreeClientContext(ISocketContext* client_ctx) = 0;
};

#endif  // EDTLS_WRAPPER_MBEDTLS_UDPSERVERBASE_H_
