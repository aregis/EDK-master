/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <huestream/HueStream.h>
#include <huestream/common/data/Color.h>
#include <huestream/effect/animation/animations/ConstantAnimation.h>
#include <huestream_example_console/Terminal.h>
#include <huestream_example_console/SawAndLightningMenu.h>
#include <huestream_example_console/BrightnessSawEffect.h>
#include <huestream_example_console/App.h>

#include <iostream>
#include <memory>

void SawAndLightningMenu::StartSaw() {
    _huestream->LockMixer();
    if (_sawEffect == nullptr) {
        _sawEffect = std::make_shared<huestream::BrightnessSawEffect>("Saw", 1);
        _sawEffect->PrepareEffect();
        _huestream->AddEffect(_sawEffect);
    }
    _sawEffect->Enable();
    _huestream->UnlockMixer();
}

void SawAndLightningMenu::StopSaw() {
    if (_sawEffect != nullptr) {
        _sawEffect->Disable();
    }
}

void SawAndLightningMenu::StartLightning() {
    _huestream->LockMixer();
    if (_lightningEffect == nullptr) {
        _lightningEffect = std::make_shared<huestream::LightningEffect>("Lightning", 2);
        _lightningEffect->PrepareEffect();
        _lightningEffect->SetOpacityAnimation(std::make_shared<huestream::ConstantAnimation>(0.75));
        _huestream->AddEffect(_lightningEffect);
    }
    _lightningEffect->Enable();
    _huestream->UnlockMixer();
}

void SawAndLightningMenu::StopLightning() {
    if (_lightningEffect != nullptr) {
        _lightningEffect->Disable();
    }
}

void SawAndLightningMenu::Run() {
    std::cout << "Saw & Lightning menu:" << std::endl;
    std::cout << "1  start saw" << std::endl;
    std::cout << "2  stop saw" << std::endl;
    std::cout << "3  start lightning" << std::endl;
    std::cout << "4  stop lightning" << std::endl;
    std::cout << "q  Quit" << std::endl;

    while (true) {
        auto key = Terminal::GetChar();

        switch (key) {
            case '1':
                StartSaw();
                break;

            case '2':
                StopSaw();
                break;

            case '3':
                StartLightning();
                break;

            case '4':
                StopLightning();
                break;

            case 'q':
                StopSaw();
                StopLightning();
                return;

            default:
                break;
        }
    }
}
SawAndLightningMenu::SawAndLightningMenu(const HueStreamPtr &hueStream) : Menu(hueStream) {}
