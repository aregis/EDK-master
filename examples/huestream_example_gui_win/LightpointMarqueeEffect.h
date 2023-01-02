/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#ifndef LIBHUESTREAM_LIGHTPOINTMARQUEEEFFECT_H
#define LIBHUESTREAM_LIGHTPOINTMARQUEEEFFECT_H

#include <huestream/effect/effects/LightIteratorEffect.h>

#include <string>
#include <memory>

namespace huestream {

    class LightpointMarqueeEffect : public LightIteratorEffect {
    public:
        LightpointMarqueeEffect(const std::string &name, unsigned int layer);

        void PrepareEffect(std::shared_ptr<ConstantAnimation> speedAnimation,
                           std::shared_ptr<ConstantAnimation> alphaAnimation);
    };
}  // namespace huestream
#endif  // LIBHUESTREAM_LIGHTPOINTMARQUEEEFFECT_H
