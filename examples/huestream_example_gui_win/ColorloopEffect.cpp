/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <huestream/effect/animation/animations/ConstantAnimation.h>
#include <huestream/effect/animation/animations/SequenceAnimation.h>
#include <huestream/effect/animation/animations/TweenAnimation.h>
#include <huestream/common/data/Area.h>
#include <huestream_example_gui_win/ColorloopEffect.h>

#include <string>
#include <memory>

namespace huestream {

    ColorloopEffect::ColorloopEffect(const std::string &name, unsigned int layer) : AreaEffect(name, layer) {}

    void ColorloopEffect::PrepareEffect(std::shared_ptr<ConstantAnimation> speedAnimation,
                                        std::shared_ptr<ConstantAnimation> alphaAnimation) {
        auto on = std::make_shared<TweenAnimation>(1, 1, 2000, TweenType::Linear);
        auto rampUp = std::make_shared<TweenAnimation>(0, 1, 2000, TweenType::Linear);
        auto rampDown = std::make_shared<TweenAnimation>(1, 0, 2000, TweenType::Linear);
        auto off = std::make_shared<TweenAnimation>(0, 0, 2000, TweenType::Linear);

        auto r = std::make_shared<SequenceAnimation>(INF);
        r->Append(on, "On");
        r->Append(rampDown, "Down");
        r->Append(off, "Off");
        r->Append(off, "Off");
        r->Append(rampUp, "Up");
        r->Append(on, "On");

        auto g = std::make_shared<SequenceAnimation>(INF);
        g->Append(rampUp, "Up");
        g->Append(on, "On");
        g->Append(on, "On");
        g->Append(rampDown, "Down");
        g->Append(off, "Off");
        g->Append(off, "Off");

        auto b = std::make_shared<SequenceAnimation>(INF);
        b->Append(off, "Off");
        b->Append(off, "Off");
        b->Append(rampUp, "Up");
        b->Append(on, "On");
        b->Append(on, "On");
        b->Append(rampDown, "Down");

        SetColorAnimation(r, g, b);
        SetOpacityAnimation(alphaAnimation);

        AddArea(Area::All);

        SetSpeedAnimation(speedAnimation);
    }

}  // namespace huestream
