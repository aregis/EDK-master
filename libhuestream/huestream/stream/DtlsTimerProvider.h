/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#ifndef HUESTREAM_STREAM_DTLSTIMERPROVIDER_H_
#define HUESTREAM_STREAM_DTLSTIMERPROVIDER_H_

#include "edtls/wrapper/mbedtls/TimerProvider.h"

#include <chrono>

namespace huestream {

class DtlsTimerProvider : public TimerProvider {
 public:
    DtlsTimerProvider();

    void set_delay(uint32_t int_ms, uint32_t fin_ms) override;

    int get_delay() override;

 protected:
    void UpdateTime();

    uint32_t _int_ms;
    uint32_t _fin_ms_;
    int64_t _now;
    int64_t _stamp;
    std::chrono::steady_clock::time_point _start;
};
}  // namespace huestream

#endif  // HUESTREAM_STREAM_DTLSTIMERPROVIDER_H_
