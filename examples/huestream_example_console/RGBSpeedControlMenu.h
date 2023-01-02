/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#ifndef HUESTREAM_EXAMPLE_CONSOLE_RGBSPEEDCONTROLMENU_H_
#define HUESTREAM_EXAMPLE_CONSOLE_RGBSPEEDCONTROLMENU_H_

#include <huestream/HueStream.h>
#include <huestream/common/data/Color.h>
#include <huestream_example_console/Terminal.h>
#include <huestream_example_console/RGBSineEffect.h>
#include <huestream_example_console/Menu.h>
#include <huestream/effect/animation/animations/ConstantAnimation.h>

#include <memory>

class RGBSpeedControlMenu: public Menu {
public:
    explicit RGBSpeedControlMenu(const huestream::HueStreamPtr &hueStream);
    void Run();

protected:
    void Start();
    void Stop();

    std::shared_ptr<huestream::RGBSineEffect> _effect;
    std::shared_ptr<huestream::ConstantAnimation> _speedAnimation;
    double _speed;
};

#endif  // HUESTREAM_EXAMPLE_CONSOLE_RGBSPEEDCONTROLMENU_H_
