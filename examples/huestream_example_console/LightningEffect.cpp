/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <huestream/effect/animation/animations/ConstantAnimation.h>
#include <huestream/effect/animation/animations/SequenceAnimation.h>
#include <huestream/effect/animation/animations/RandomAnimation.h>
#include <huestream/effect/animation/animations/TweenAnimation.h>
#include <huestream_example_console/LightningEffect.h>

#include <string>
#include <memory>

namespace huestream {

LightningEffect::LightningEffect(const std::string &name, unsigned int layer) : LightSourceEffect(name, layer) {}

void LightningEffect::PrepareEffect() {
        auto i = std::make_shared<RandomAnimation>(0, 1, 100, 300, TweenType::Linear);

        SetColorAnimation(i, i, i);
        SetRadiusAnimation(i);

        auto x = std::make_shared<RandomAnimation>(-1, 1, 600, 1200, TweenType::Linear);
        auto y = std::make_shared<RandomAnimation>(-1, 1, 600, 1200, TweenType::Linear);
        SetPositionAnimation(x, y);
}
}  // namespace huestream
