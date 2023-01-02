/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <huestream/effect/animation/animations/ConstantAnimation.h>
#include <huestream/effect/animation/animations/SequenceAnimation.h>
#include <huestream/effect/animation/animations/TweenAnimation.h>
#include <huestream/common/data/Area.h>
#include <huestream_example_console/BrightnessSawEffect.h>

#include <string>
#include <memory>

namespace huestream {

    BrightnessSawEffect::BrightnessSawEffect(const std::string &name, unsigned int layer) :
        AreaEffect(name, layer) {}

    void BrightnessSawEffect::PrepareEffect() {
        auto r = std::make_shared<SequenceAnimation>(INF);

        auto rampUp = std::make_shared<TweenAnimation>(0, 1, 5000, TweenType::Linear);
        r->Append(rampUp, "Up");

        auto rampDown = std::make_shared<TweenAnimation>(1, 0, 5000, TweenType::Linear);
        r->Append(rampDown, "Down");

        auto g = std::make_shared<ConstantAnimation>(0);
        auto b = std::make_shared<ConstantAnimation>(0);

        SetColorAnimation(r, g, b);

        AddArea(Area::FrontHalf);
    }
}  // namespace huestream
