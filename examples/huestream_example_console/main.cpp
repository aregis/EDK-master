/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <huestream_example_console/Terminal.h>
#include <support/logging/Log.h>
#include <huestream/config/Config.h>
#include <huestream/HueStream.h>
#include <huestream_example_console/App.h>
#include <huestream_example_console/ExplosionMenu.h>
#include <huestream_example_console/SequenceMenu.h>
#include <huestream_example_console/SawAndLightningMenu.h>
#include <huestream_example_console/RGBSpeedControlMenu.h>

#include <stdio.h>

#include <thread>
#include <iostream>

using std::cout;
using std::endl;

class ConnectionFlowExample : public App {
public:
    ConnectionFlowExample() : App("HueConsoleExample", "PC") {}

    void StartExample() override {
        MainMenu();
    }

    void MainMenu() const {
        auto print = true;
        while (true) {
            if (print) {
                cout << "Main menu:" << endl;
                cout << "e  Explosion demo" << endl;
                cout << "s  Sequence demo" << endl;
                cout << "l  Saw and Lightning demo" << endl;
                cout << "r  RGB Speed Control demo" << endl;
                cout << "q  Quit" << endl;
                print = false;
            }
            auto key = Terminal::GetChar();
            if (key == 'e') {
                ExplosionMenu(_huestream).Run();
                print = true;
            } else if (key == 'l') {
                SawAndLightningMenu(_huestream).Run();
                print = true;
            } else if (key == 's') {
                SequenceMenu(_huestream).Run();
                print = true;
            } else if (key == 'r') {
                RGBSpeedControlMenu(_huestream).Run();
                print = true;
            } else if (key == 'q') {
                break;
            }
        }
    }
};

int main(int argc, char *argv[]) {
    auto program = ConnectionFlowExample();
    program.Main(argc, argv);
    return 0;
}
