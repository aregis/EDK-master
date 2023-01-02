/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <huestream/config/Config.h>
#include <huestream/HueStream.h>
#include <huestream/common/util/Rand.h>
#include <huestream/effect/effects/ExplosionEffect.h>
#include <huestream/common/data/Area.h>
#include <huestream_example_console/Terminal.h>
#include <huestream_example_console/ExplosionMenu.h>
#include <huestream_example_console/App.h>

#include <string>
#include <iostream>
#include <memory>

using huestream::HueStream;
using huestream::ExplosionEffect;
using huestream::AreaEffect;
using huestream::Color;
using std::cout;
using std::endl;

ExplosionMenu::ExplosionMenu(const HueStreamPtr &hueStream) : Menu(hueStream) {}

void ExplosionMenu::PlayExplosion(double x, double y) {
    static auto explosionCount = 0;
    _huestream->LockMixer();
    std::string name = "BAM!" + std::to_string(explosionCount++);
    auto explosion = std::make_shared<ExplosionEffect>(name, 1);
    auto color = Color(1, 0.8, 0.4);
    auto radius = 0.5;
    auto intensity_expansion_time = 50;
    auto radius_expansion_time = 106;
    auto duration = 2000;

    explosion->PrepareEffect(color, huestream::Location(x, y), duration, radius, radius_expansion_time, intensity_expansion_time);
    _huestream->AddEffect(explosion);
    explosion->Enable();
    _huestream->UnlockMixer();
}

void ExplosionMenu::Run() {
    auto x = 0.0;
    auto y = 0.0;
    std::cout << "Explosion menu:" << std::endl;
    std::cout << "r  Explosion at random location" << std::endl;
    std::cout << "space  Explosion at 'current location'" << std::endl;
    std::cout << "w  Move 'current location' up" << std::endl;
    std::cout << "s  Move 'current location' down" << std::endl;
    std::cout << "a  Move 'current location' left" << std::endl;
    std::cout << "d  Move 'current location' right" << std::endl;
    std::cout << "b  Toggle white background" << std::endl;
    std::cout << "q  Quit" << std::endl;

    auto background = CreateBackground();

    while (true) {
        auto key = Terminal::GetChar();

        switch (key) {
            case 'r': {
                cout << "Playing Explosion effect random" << endl;
                PlayExplosion(huestream::Rand(-1, 1), huestream::Rand(-1, 1));
                break;
            }
            case ' ': {
                cout << "Playing Explosion effect at (" << x << ", " << y << ")" << endl;
                PlayExplosion(x, y);
                break;
            }
            case 'w': {
                y += .1;
                cout << "Moving Explosion to (" << x << ", " << y << ")" << endl;
                break;
            }
            case 's': {
                y -= .1;
                cout << "Moving Explosion to (" << x << ", " << y << ")" << endl;
                break;
            }
            case 'a': {
                x -= .1;
                cout << "Moving Explosion to (" << x << ", " << y << ")" << endl;
                break;
            }
            case 'd': {
                x += .1;
                cout << "Moving Explosion to (" << x << ", " << y << ")" << endl;
                break;
            }
            case 'b': {
                background->IsEnabled() ? background->Disable() : background->Enable();
                break;
            }
            case 'q': {
                return;
            }
            default: {
                break;
            }
        }
    }
}
shared_ptr<AreaEffect> ExplosionMenu::CreateBackground() const {
    _huestream->LockMixer();
    auto background = std::make_shared<AreaEffect>("background", 0);
    background->SetArea(huestream::Area::All);
    auto color = Color(0.2, 0.2, 0.2);
    background->SetFixedColor(color);
    _huestream->AddEffect(background);
    _huestream->UnlockMixer();
    return background;
}

