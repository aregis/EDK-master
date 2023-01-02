/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <string>
#include <sstream>
#include <vector>
#include <cstring>
#include <cstdio>

#define MAKE_STRING_BASIC(x) #x
#define MAKE_STRING(x) MAKE_STRING_BASIC(x)

class HueArguments {
public:
    HueArguments(int argc, char* argv[]) {
        std::vector<std::string>  hue_argument_list;

#ifdef HUE_ARGUMENTS
        std::stringstream ss{MAKE_STRING(HUE_ARGUMENTS)};
        std::string tok;
        while (std::getline(ss, tok, ' ')) {
            hue_argument_list.push_back(tok);
        }
#endif

        _argc = argc + static_cast<int>(hue_argument_list.size());
        _argv = new char*[_argc + 1];
        _argv[_argc] = nullptr;

        _argv_internal = new char*[_argc];

        auto clone_argument = [&](int index, const char* argument) {
            const auto argument_length = strlen(argument) + 1;
            _argv_internal[index] = _argv[index] = new char[argument_length];
#ifdef _WIN32
            _snprintf_s(_argv[index], argument_length, argument_length, "%s", argument);
#else
            snprintf(_argv[index], argument_length, "%s", argument);
#endif
        };

        for (int i = 0; i < argc; ++i) {
            clone_argument(i, argv[i]);
        }

        for (unsigned int i = 0; i < hue_argument_list.size(); ++i) {
            clone_argument(argc + i, hue_argument_list[i].data());
        }
    }

    ~HueArguments() {
        for (int i = 0; i < _argc; ++i) {
            delete [] _argv_internal[i];
        }
        delete[] _argv;
        delete[] _argv_internal;
    }

    int argc() const {
        return _argc;
    }

    char** argv() const {
        return _argv;
    }

private:
    int _argc = 0;
    char** _argv = nullptr;
    char** _argv_internal = nullptr;
};

