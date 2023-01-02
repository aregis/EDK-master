/*******************************************************************************
Copyright © 2016 Philips Lighting Holding B.V.
All Rights Reserved.
********************************************************************************/

#ifndef EDTLS_WRAPPER_MBEDTLS_MBEDTLSSERVERPLATFORM_H_
#define EDTLS_WRAPPER_MBEDTLS_MBEDTLSSERVERPLATFORM_H_

#include <edtls/wrapper/IPSKProvider.h>
#include <edtls/wrapper/mbedtls/MbedtlsPlatform.h>

#include <memory>

class MbedtlsServerPlatform: public MbedtlsPlatform {
 public:
  MbedtlsServerPlatform(LogFunction log_function,
                          EntropyProviderBasePtr entropy_provider,
                          TimerProviderPtr timer_provider,
                          PSKProviderPtr psk_provider);

  PSKProviderPtr get_psk_provider();

 protected:
  PSKProviderPtr psk_provider_;
};
typedef std::shared_ptr<MbedtlsServerPlatform> MbedtlsServerPlatformPtr;

#endif  // EDTLS_WRAPPER_MBEDTLS_MBEDTLSSERVERPLATFORM_H_
