/*******************************************************************************
Copyright © 2016 Philips Lighting Holding B.V.
All Rights Reserved.
********************************************************************************/

#ifndef EDTLS_WRAPPER_MBEDTLS_DEFAULTUDPCLIENT_H_
#define EDTLS_WRAPPER_MBEDTLS_DEFAULTUDPCLIENT_H_

#include <edtls/wrapper/mbedtls/UDPClientBase.h>
#include <string>

class DefaultUDPClient: public UDPBlocking<UDPClientBase> {
 private:
  mbedtls_net_context socket_;
 public:
  virtual ~DefaultUDPClient() = default;
  netstatus_e Connect(const std::string &host, unsigned int port) override;
  void Init() override;
  void Free() override;
  int Send(const unsigned char *buf, size_t len) override;
 private:
  int ReceiveWithTimeout(unsigned char *buf, size_t len, uint32_t timeout) override;
};

#endif  // EDTLS_WRAPPER_MBEDTLS_DEFAULTUDPCLIENT_H_
