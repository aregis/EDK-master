/*******************************************************************************
Copyright © 2016 Philips Lighting Holding B.V.
All Rights Reserved.
********************************************************************************/

#include <edtls/wrapper/mbedtls/DefaultTimerProvider.h>

DefaultTimerProvider::DefaultTimerProvider() :
    _int_ms(0),
    _fin_ms(0),
    _now(0),
    _start(std::chrono::steady_clock::now()),
    _stamp(0) {
}

void DefaultTimerProvider::UpdateTime() {
  _now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - _start).count();
}

void DefaultTimerProvider::set_delay(uint32_t int_ms, uint32_t fin_ms) {
  UpdateTime();
  _int_ms = int_ms;
  _fin_ms = fin_ms;
  _stamp = _now;
}

int DefaultTimerProvider::get_delay() {
  UpdateTime();
  auto elapsed_ms = _now - _stamp;

  if (_fin_ms == 0)
    return (-1);

  if (elapsed_ms >= _fin_ms)
    return (2);

  if (elapsed_ms >= _int_ms)
    return (1);

  return (0);
}
