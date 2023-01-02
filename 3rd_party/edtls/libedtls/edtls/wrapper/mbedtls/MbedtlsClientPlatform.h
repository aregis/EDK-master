/*******************************************************************************
Copyright © 2016 Philips Lighting Holding B.V.
All Rights Reserved.
********************************************************************************/

#ifndef EDTLS_WRAPPER_MBEDTLS_MBEDTLSCLIENTPLATFORM_H_
#define EDTLS_WRAPPER_MBEDTLS_MBEDTLSCLIENTPLATFORM_H_

#include <edtls/wrapper/mbedtls/UDPClientBase.h>
#include <edtls/wrapper/mbedtls/MbedtlsPlatform.h>

#include <memory>

class MbedtlsClientPlatform : public MbedtlsPlatform {
 public:
  virtual ~MbedtlsClientPlatform() = default;
  MbedtlsClientPlatform(LogFunction log_function,
                          EntropyProviderBasePtr entropy_provider,
                          TimerProviderPtr timer_provider,
                          UDPClientBasePtr udpClient);
  UDPClientBasePtr get_udp_client();
 protected:
  UDPClientBasePtr udpClient_;
};

typedef std::shared_ptr<MbedtlsClientPlatform> MbedtlsClientPlatformPtr;

#endif  // EDTLS_WRAPPER_MBEDTLS_MBEDTLSCLIENTPLATFORM_H_
