#include <windows.h>

namespace timer {
    class hardwaretimer {
    public:
        LARGE_INTEGER StartingTime, EndingTime, ElapsedMicroseconds;
        LARGE_INTEGER Frequency;

        hardwaretimer();

        void Getstarttime(void);

        void GetEndtime(void);

        LONGLONG Getintervel(void);

    };
}
