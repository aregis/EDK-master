#include "testwrapper.h"

using namespace std;

namespace wrapper {

    string testwrapper::ExePath() {
        char buffer[MAX_PATH];
        GetModuleFileName(NULL, buffer, MAX_PATH);
        string::size_type pos = string(buffer).find_last_of("\\/");
        return string(buffer).substr(0, pos);
    }

    void testwrapper::pythonrun(string filename) {
        std::string workPath = ExePath();
        std::string Dirfilename = workPath + filename;
        std::string command = "python ";
        command += Dirfilename;
        system(command.c_str());
        std::cin.ignore();
        return;
    }

}






