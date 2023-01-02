/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#ifndef HUESTREAM_EXAMPLE_CONSOLE_MENU_H_
#define HUESTREAM_EXAMPLE_CONSOLE_MENU_H_

#include <huestream/HueStream.h>

class Menu {
public:
    explicit Menu(huestream::HueStreamPtr hueStream);
    virtual void Run() = 0;
 protected:
    huestream::HueStreamPtr _huestream;
};

#endif  // HUESTREAM_EXAMPLE_CONSOLE_MENU_H_
