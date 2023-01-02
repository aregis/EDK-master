/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <huestream/config/Config.h>
#include <huestream/HueStream.h>
#include <huestream/effect/effects/SequenceEffect.h>
#include <huestream_example_console/Terminal.h>
#include <huestream_example_console/SequenceMenu.h>
#include <huestream_example_console/App.h>

#include <string>
#include <memory>
#include <iostream>

using huestream::HueStream;
using huestream::SequenceEffect;
using huestream::SequenceMode;
using huestream::Color;

void SequenceMenu::Run() {
    std::cout << "Sequence menu:" << std::endl;
    std::cout << "1  Sets sequence to SequenceModeAll" << std::endl;
    std::cout << "2  Sets sequence to SequenceModeRound" << std::endl;
    std::cout << "3  Sets sequence to SequenceModeFrontBack" << std::endl;
    std::cout << "4  Sets sequence to SequenceModeLeftRight" << std::endl;
    std::cout << "R  Adds red to the sequence" << std::endl;
    std::cout << "G  Adds green to the sequence" << std::endl;
    std::cout << "B  Adds blue to the sequence" << std::endl;

    std::cout << "space  Do step in sequence'" << std::endl;
    std::cout << "-  Decrease brightness" << std::endl;
    std::cout << "=  Increase brightness" << std::endl;
    std::cout << "q  Quit" << std::endl;

    auto _effect = std::make_shared<SequenceEffect>("seq", 1);
    _huestream->AddEffect(_effect);
    _effect->Enable();
    while (true) {
        auto c = Terminal::GetChar();
        switch (c) {
            case '1':
                _effect->SetMode(huestream::SequenceModeAll);
                break;
            case '2':
                _effect->SetMode(huestream::SequenceModeRound);
                break;
            case '3':
                _effect->SetMode(huestream::SequenceModeFrontBack);
                break;
            case '4':
                _effect->SetMode(huestream::SequenceModeLeftRight);
                break;
            case 'r':
                _effect->AddColor(Color(1.0, 0.0, 0.0));
                break;
            case 'g':
                _effect->AddColor(Color(0.0, 1.0, 0.0));
                break;
            case 'b':
                _effect->AddColor(Color(0.0, 0.0, 1.0));
                break;
            case ' ':
                _effect->Step();
                break;
            case '=':
                _effect->InceaseBri(0.1);
                break;
            case '-':
                _effect->DecreaseBri(0.1);
                break;
            default:
                break;
        }
        if (c == 'q') {
            break;
        }
    }
    _effect->Finish();
}
SequenceMenu::SequenceMenu(const HueStreamPtr &hueStream) : Menu(hueStream) {}


