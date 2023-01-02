/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#ifndef HUESTREAM_EXAMPLE_GUI_WIN_BRIGHTNESSSAWEFFECT_H_
#define HUESTREAM_EXAMPLE_GUI_WIN_BRIGHTNESSSAWEFFECT_H_

#include <huestream/effect/Mixer.h>
#include <huestream/effect/effects/AreaEffect.h>
#include <huestream/effect/animation/animations/ConstantAnimation.h>

#include <string>
#include <memory>

namespace huestream {
class BrightnessSawEffect : public AreaEffect {
    public:
        BrightnessSawEffect(const std::string &name, unsigned int layer);

        void PrepareEffect(std::shared_ptr<ConstantAnimation> speedAnimation,
                           std::shared_ptr<ConstantAnimation> alphaAnimation);
    };
}  // namespace huestream

#endif  // HUESTREAM_EXAMPLE_GUI_WIN_BRIGHTNESSSAWEFFECT_H_
