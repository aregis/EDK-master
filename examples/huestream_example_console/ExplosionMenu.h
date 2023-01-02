/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#ifndef HUESTREAM_EXAMPLE_CONSOLE_EXPLOSIONMENU_H_
#define HUESTREAM_EXAMPLE_CONSOLE_EXPLOSIONMENU_H_

#include <huestream/effect/effects/AreaEffect.h>
#include <huestream_example_console/Menu.h>

#include <memory>

class ExplosionMenu : public Menu {
public:
    explicit ExplosionMenu(const huestream::HueStreamPtr &hueStream);
    virtual void Run();

 protected:
    void PlayExplosion(double x, double y);
    shared_ptr<huestream::AreaEffect> CreateBackground() const;
};

#endif  // HUESTREAM_EXAMPLE_CONSOLE_EXPLOSIONMENU_H_
