/*******************************************************************************
Copyright © 2016 Philips Lighting Holding B.V.
All Rights Reserved.
********************************************************************************/

#include <edtls/wrapper/mbedtls/MbedtlsClientPlatform.h>

MbedtlsClientPlatform::MbedtlsClientPlatform(LogFunction log_function,
                                             EntropyProviderBasePtr entropy_provider,
                                             TimerProviderPtr timer_provider,
                                             UDPClientBasePtr udpClient)
    : MbedtlsPlatform(log_function, entropy_provider, timer_provider), udpClient_(udpClient) {
}

UDPClientBasePtr MbedtlsClientPlatform::get_udp_client() {
  return udpClient_;
}
