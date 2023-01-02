/*******************************************************************************
Copyright © 2016 Philips Lighting Holding B.V.
All Rights Reserved.
********************************************************************************/

#ifndef EDTLS_WRAPPER_MBEDTLS_UDPCLIENTBASE_H_
#define EDTLS_WRAPPER_MBEDTLS_UDPCLIENTBASE_H_

#include <edtls/wrapper/mbedtls/UDPBase.h>

#include <memory>
#include <string>

class UDPClientBase: public UDPBase {
 public:
  virtual ~UDPClientBase() = default;
  virtual netstatus_e Connect(const std::string &host, unsigned int port) = 0;
};

typedef std::shared_ptr<UDPClientBase> UDPClientBasePtr;

#endif  // EDTLS_WRAPPER_MBEDTLS_UDPCLIENTBASE_H_
