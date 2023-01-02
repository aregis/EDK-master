/*******************************************************************************
Copyright © 2016 Philips Lighting Holding B.V.
All Rights Reserved.
********************************************************************************/

#ifndef EDTLS_WRAPPER_MBEDTLS_TIMERPROVIDER_H_
#define EDTLS_WRAPPER_MBEDTLS_TIMERPROVIDER_H_

#include <memory>

class TimerProvider {
 public:
  virtual ~TimerProvider() {}
  virtual void set_delay(uint32_t int_ms, uint32_t fin_ms) = 0;
  virtual int get_delay() = 0;
};

typedef std::shared_ptr<TimerProvider> TimerProviderPtr;

#endif  // EDTLS_WRAPPER_MBEDTLS_TIMERPROVIDER_H_
