/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <huestream/config/Config.h>
#include <huestream/effect/animation/animations/ConstantAnimation.h>
#include <huestream/effect/animation/animations/SequenceAnimation.h>
#include <huestream/effect/animation/animations/TweenAnimation.h>
#include <huestream_example_gui_win/LightpointMarqueeEffect.h>

#include <string>
#include <memory>

namespace huestream {

    LightpointMarqueeEffect::LightpointMarqueeEffect(const std::string &name, unsigned int layer) : LightIteratorEffect(
            name, layer) {}

    void LightpointMarqueeEffect::PrepareEffect(std::shared_ptr<ConstantAnimation> speedAnimation,
                                                std::shared_ptr<ConstantAnimation> alphaAnimation) {
        auto time = 250;
        auto maxBrightness = 0.5;
        auto timesOnOneLight = 3;
        auto offset = time * 2 * timesOnOneLight;

        auto r = std::make_shared<SequenceAnimation>(timesOnOneLight - 1);
        auto g = std::make_shared<SequenceAnimation>(timesOnOneLight - 1);
        auto b = std::make_shared<SequenceAnimation>(timesOnOneLight - 1);

        auto rampUp = std::make_shared<TweenAnimation>(0, maxBrightness, time, TweenType::Linear);
        auto rampDown = std::make_shared<TweenAnimation>(maxBrightness, 0, time, TweenType::Linear);

        r->Append(rampUp, "UpRed");
        r->Append(rampDown, "DownRed");

        g->Append(rampUp, "UpGreen");
        g->Append(rampDown, "DownGreen");

        b->Append(rampUp, "UpBlue");
        b->Append(rampDown, "DownBlue");

        SetColorAnimation(r, g, b);

        alphaAnimation->SetLength(offset);
        SetOpacityAnimation(alphaAnimation);

        SetSpeedAnimation(speedAnimation);

        SetMode(IterationModeCycle);
        SetOrder(IterationOrderClockwise);
        SetOffset(offset);
    }

}  // namespace huestream
