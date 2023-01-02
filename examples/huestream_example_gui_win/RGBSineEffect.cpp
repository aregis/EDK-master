/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <huestream/effect/animation/animations/ConstantAnimation.h>
#include <huestream/effect/animation/animations/SequenceAnimation.h>
#include <huestream/effect/animation/animations/TweenAnimation.h>
#include <huestream_example_gui_win/RGBSineEffect.h>

#include <string>
#include <memory>

namespace huestream {

    RGBSineEffect::RGBSineEffect(const std::string &name, unsigned int layer) :
        LightSourceEffect(name, layer) {}

    void RGBSineEffect::PrepareEffect(std::shared_ptr<ConstantAnimation> speedAnimation,
                                      std::shared_ptr<ConstantAnimation> alphaAnimation) {
        auto time = 1000;
        auto maxBrightness = 1;

        auto g = std::make_shared<SequenceAnimation>(INF);
        auto r = std::make_shared<SequenceAnimation>(INF);
        auto b = std::make_shared<SequenceAnimation>(INF);

        auto rampUp = std::make_shared<TweenAnimation>(0, maxBrightness, time, TweenType::EaseInOutSine);
        auto rampDown = std::make_shared<TweenAnimation>(maxBrightness, 0, time, TweenType::EaseInOutSine);
        auto off = std::make_shared<TweenAnimation>(0, 0, 2 * time, TweenType::Linear);

        g->Append(rampUp, "UpGreenA");
        g->Append(rampDown, "DownGreenA");
        r->Append(off, "OffRedA");
        b->Append(off, "OffBlueA");

        g->Append(off, "OffGreenB");
        r->Append(rampUp, "UpRedB");
        r->Append(rampDown, "DownRedB");
        b->Append(off, "OffBlueB");

        g->Append(off, "OffGreenC");
        r->Append(off, "OffRedC");
        b->Append(rampUp, "UpBlueC");
        b->Append(rampDown, "DownBlueC");

        SetColorAnimation(r, g, b);
        SetOpacityAnimation(alphaAnimation);
        auto radius = std::make_shared<ConstantAnimation>(10);
        SetRadiusAnimation(radius);
        SetSpeedAnimation(speedAnimation);
    }

}  // namespace huestream
