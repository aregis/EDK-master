/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <huestream/stream/DtlsTimerProvider.h>

namespace huestream {

DtlsTimerProvider::DtlsTimerProvider() :
        _int_ms(0),
        _fin_ms_(0),
        _now(0),
        _stamp(0),
        _start(std::chrono::steady_clock::now()) {
}

void DtlsTimerProvider::UpdateTime() {
    _now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - _start).count();
}

void DtlsTimerProvider::set_delay(uint32_t int_ms, uint32_t fin_ms) {
    UpdateTime();
    _int_ms = int_ms;
    _fin_ms_ = fin_ms;
    _stamp = _now;
}

int DtlsTimerProvider::get_delay() {
    UpdateTime();
    auto elapsed_ms = _now - _stamp;

    if (_fin_ms_ == 0)
        return (-1);

    if (elapsed_ms >= _fin_ms_)
        return (2);

    if (elapsed_ms >= _int_ms)
        return (1);

    return (0);
}
}  // namespace huestream
