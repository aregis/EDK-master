/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#ifndef HUESTREAM_EXAMPLE_CONSOLE_LIGHTNINGEFFECT_H_
#define HUESTREAM_EXAMPLE_CONSOLE_LIGHTNINGEFFECT_H_

#include <huestream/effect/effects/LightSourceEffect.h>

#include <string>

namespace huestream {

    class LightningEffect : public LightSourceEffect {
    public:
        LightningEffect(const std::string &name, unsigned int layer);

        void PrepareEffect();
    };
}  // namespace huestream
#endif  // HUESTREAM_EXAMPLE_CONSOLE_LIGHTNINGEFFECT_H_
