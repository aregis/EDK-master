/*******************************************************************************
Copyright © 2016 Philips Lighting Holding B.V.
All Rights Reserved.
********************************************************************************/

#include <edtls/wrapper/mbedtls/MbedtlsPlatform.h>
#include <edtls/wrapper/mbedtls/TimerProvider.h>

MbedtlsPlatform::MbedtlsPlatform(LogFunction log_function,
                                 EntropyProviderBasePtr entropy_provider,
                                 TimerProviderPtr timer_provider) :
    entropy_provider_(entropy_provider),
    timer_provider_(timer_provider),
    log_function_(log_function) {
}

LogFunction MbedtlsPlatform::get_log_function() {
  return log_function_;
}

EntropyProviderBasePtr MbedtlsPlatform::get_entropy_provider() {
  return entropy_provider_;
}

TimerProviderPtr MbedtlsPlatform::get_timer_provider() {
  return timer_provider_;
}

