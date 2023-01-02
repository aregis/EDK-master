/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#ifndef HUESTREAM_EXAMPLE_CONSOLE_SAWANDLIGHTNINGMENU_H_
#define HUESTREAM_EXAMPLE_CONSOLE_SAWANDLIGHTNINGMENU_H_

#include <huestream/HueStream.h>
#include <huestream/common/data/Color.h>
#include <huestream_example_console/Terminal.h>
#include <huestream_example_console/BrightnessSawEffect.h>
#include <huestream_example_console/LightningEffect.h>
#include <huestream_example_console/Menu.h>

#include <memory>

class SawAndLightningMenu: public Menu {
public:
    explicit SawAndLightningMenu(const huestream::HueStreamPtr &hueStream);

    void Run();

protected:
    void StartSaw();

    void StopSaw();

    void StartLightning();

    void StopLightning();

    std::shared_ptr<huestream::BrightnessSawEffect> _sawEffect;
    std::shared_ptr<huestream::LightningEffect> _lightningEffect;
};

#endif  // HUESTREAM_EXAMPLE_CONSOLE_SAWANDLIGHTNINGMENU_H_
