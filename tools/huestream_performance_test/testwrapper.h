#include <string>
#include <ctime>
#include <sstream>
#include <string>
#include <fstream>
#include <iostream>
#include <windows.h>


using namespace std;

namespace wrapper {
    class testwrapper {
    public:
        static string ExePath();

        static void pythonrun(string filename);
    };
}

