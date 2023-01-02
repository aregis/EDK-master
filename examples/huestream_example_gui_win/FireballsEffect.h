/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#ifndef LIBHUESTREAM_FIREBALLSEFFECT_H
#define LIBHUESTREAM_FIREBALLSEFFECT_H

#include <huestream/effect/effects/LightSourceEffect.h>
#include <huestream/effect/animation/animations/ConstantAnimation.h>

#include <string>
#include <memory>

namespace huestream {

    class FireballsEffect : public LightSourceEffect {
    public:
        FireballsEffect(const std::string &name, unsigned int layer);

        void PrepareEffect(std::shared_ptr<ConstantAnimation> speedAnimation,
                           std::shared_ptr<ConstantAnimation> alphaAnimation);
    };
}  // namespace huestream
#endif  // LIBHUESTREAM_LIGHTPOINTMARQUEEEFFECT_H
