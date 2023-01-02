/*******************************************************************************
Copyright © 2016 Philips Lighting Holding B.V.
All Rights Reserved.
********************************************************************************/

#ifndef EDTLS_WRAPPER_MBEDTLS_MBEDTLSPLATFORM_H_
#define EDTLS_WRAPPER_MBEDTLS_MBEDTLSPLATFORM_H_

#include <edtls/wrapper/mbedtls/EntropyProviderBase.h>
#include <edtls/logger/Logger.h>
#include <edtls/wrapper/mbedtls/TimerProvider.h>
#include <memory>

class MbedtlsPlatform {
 public:
  virtual ~MbedtlsPlatform() = default;
  MbedtlsPlatform(LogFunction log_function,
                  EntropyProviderBasePtr entropy_provider,
                  TimerProviderPtr timer_provider);
  EntropyProviderBasePtr get_entropy_provider();
  LogFunction get_log_function();
  TimerProviderPtr get_timer_provider();
 protected:
  EntropyProviderBasePtr entropy_provider_;
  TimerProviderPtr timer_provider_;
  LogFunction log_function_;
};

#endif  // EDTLS_WRAPPER_MBEDTLS_MBEDTLSPLATFORM_H_
