/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#ifndef HUESTREAM_EXAMPLE_CONSOLE_BRIGHTNESSSAWEFFECT_H_
#define HUESTREAM_EXAMPLE_CONSOLE_BRIGHTNESSSAWEFFECT_H_

#include <huestream/effect/effects/AreaEffect.h>

#include <string>

namespace huestream {

    class BrightnessSawEffect : public AreaEffect {
    public:
        BrightnessSawEffect(const std::string &name, unsigned int layer);

        void PrepareEffect();
    };
}  // namespace huestream
#endif  // HUESTREAM_EXAMPLE_CONSOLE_BRIGHTNESSSAWEFFECT_H_
