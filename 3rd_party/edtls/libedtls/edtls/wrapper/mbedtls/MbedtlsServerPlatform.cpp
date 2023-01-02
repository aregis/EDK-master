/*******************************************************************************
Copyright © 2016 Philips Lighting Holding B.V.
All Rights Reserved.
********************************************************************************/

#include <edtls/wrapper/mbedtls/MbedtlsServerPlatform.h>

MbedtlsServerPlatform::MbedtlsServerPlatform(LogFunction log_function,
                                             EntropyProviderBasePtr entropy_provider,
                                             TimerProviderPtr timer_provider,
                                             PSKProviderPtr pskProvider) :
    MbedtlsPlatform(log_function, entropy_provider, timer_provider), psk_provider_(pskProvider) {
}

PSKProviderPtr MbedtlsServerPlatform::get_psk_provider() {
  return psk_provider_;
}
