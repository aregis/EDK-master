/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#ifndef HUESTREAM_EXAMPLE_CONSOLE_RGBSINEEFFECT_H_
#define HUESTREAM_EXAMPLE_CONSOLE_RGBSINEEFFECT_H_

#include <huestream/effect/effects/LightSourceEffect.h>

#include <string>

namespace huestream {

    class RGBSineEffect : public LightSourceEffect {
    public:
        explicit RGBSineEffect(const std::string &name = "", unsigned int layer = 0);

        void PrepareEffect();
    };
}  // namespace huestream
#endif  // HUESTREAM_EXAMPLE_CONSOLE_RGBSINEEFFECT_H_
