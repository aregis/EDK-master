#include <fstream>
#include <iostream>
#include <ctime>
#include <string.h>
#include <windows.h>
#include "hardwaretimer.h"
#include <sstream>

#pragma warning(disable : 4996)
#define DEFAULT_BUFLEN 8

using namespace timer;

namespace server {
    class testserver {
    public:
        static void serverinit(hardwaretimer *timmer, bool *connectionFlag);
    };
}
