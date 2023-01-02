/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <huestream/HueStream.h>
#include <huestream/common/data/Color.h>
#include <huestream_example_console/Terminal.h>
#include <huestream_example_console/RGBSpeedControlMenu.h>
#include <huestream_example_console/App.h>

#include <iostream>
#include <memory>

RGBSpeedControlMenu::RGBSpeedControlMenu(const HueStreamPtr &hueStream) : Menu(hueStream) {}


void RGBSpeedControlMenu::Start() {
    _speed = 1;
    _speedAnimation = std::make_shared<huestream::ConstantAnimation>(_speed);
    _huestream->LockMixer();
    if (_effect == nullptr) {
        _effect = std::make_shared<huestream::RGBSineEffect>("Sine", 1);
        _effect->PrepareEffect();
        _huestream->AddEffect(_effect);
    }
    _effect->SetSpeedAnimation(_speedAnimation);
    _effect->Enable();
    _huestream->UnlockMixer();
}

void RGBSpeedControlMenu::Stop() {
    if (_effect != nullptr) {
        _effect->Disable();
    }
}

void RGBSpeedControlMenu::Run() {
    std::cout << "RGB Sine speed menu:" << std::endl;
    std::cout << "1  start" << std::endl;
    std::cout << "2  stop" << std::endl;
    std::cout << "n  Faster" << std::endl;
    std::cout << "m  Slower" << std::endl;
    std::cout << "q  Quit" << std::endl;

    while (true) {
        auto key = Terminal::GetChar();

        switch (key) {
            case '1':
                Start();
                break;

            case '2':
                Stop();
                break;

            case 'n':
                _speed += 0.1;
                _speedAnimation->SetValue(_speed);
                std::cout << "Speed is " << _speed << std::endl;
                break;

            case 'm':
                if (_speed > 0.1) {
                    _speed -= 0.1;
                }
                _speedAnimation->SetValue(_speed);
                std::cout << "Speed is " << _speed << std::endl;
                break;

            case 'q':
                Stop();
                return;

            default:
                break;
        }
    }
}
