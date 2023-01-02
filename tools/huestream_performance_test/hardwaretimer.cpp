#include "hardwaretimer.h"

namespace timer {
    hardwaretimer::hardwaretimer() {
        QueryPerformanceFrequency(&Frequency);
    }

    void hardwaretimer::Getstarttime() {
        QueryPerformanceCounter(&StartingTime);
    }

    void hardwaretimer::GetEndtime() {
        QueryPerformanceCounter(&EndingTime);
    }

    LONGLONG hardwaretimer::Getintervel() {

        ElapsedMicroseconds.QuadPart = EndingTime.QuadPart - StartingTime.QuadPart;
        ElapsedMicroseconds.QuadPart *= 1000000;
        ElapsedMicroseconds.QuadPart /= Frequency.QuadPart;
        return ElapsedMicroseconds.QuadPart;
    }
}