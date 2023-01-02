/*******************************************************************************
Copyright © 2016 Philips Lighting Holding B.V.
All Rights Reserved.
********************************************************************************/

#ifndef EDTLS_WRAPPER_MBEDTLS_DEFAULTTIMERPROVIDER_H_
#define EDTLS_WRAPPER_MBEDTLS_DEFAULTTIMERPROVIDER_H_

#include <edtls/wrapper/mbedtls/TimerProvider.h>

#include <chrono>

class DefaultTimerProvider : public TimerProvider {
 public:
  DefaultTimerProvider();

  void set_delay(uint32_t int_ms, uint32_t fin_ms) override;

  int get_delay() override;

 protected:
  void UpdateTime();

  uint32_t _int_ms;
  uint32_t _fin_ms;
  int64_t _now;
  std::chrono::steady_clock::time_point _start;
  int64_t _stamp;
};

#endif  // EDTLS_WRAPPER_MBEDTLS_DEFAULTTIMERPROVIDER_H_
