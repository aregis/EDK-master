/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <huestream/effect/animation/animations/ConstantAnimation.h>
#include <huestream/effect/animation/animations/SequenceAnimation.h>
#include <huestream/effect/animation/animations/TweenAnimation.h>
#include <huestream/common/data/CuboidArea.h>
#include <huestream_example_gui_win/BrightnessSawEffect.h>

#include <string>
#include <memory>

using std::string;

namespace huestream {

    BrightnessSawEffect::BrightnessSawEffect(const string &name, unsigned int layer) : AreaEffect(name, layer) {}

    void BrightnessSawEffect::PrepareEffect(std::shared_ptr<ConstantAnimation> speedAnimation,
                                            std::shared_ptr<ConstantAnimation> alphaAnimation) {
        auto r = std::make_shared<SequenceAnimation>(INF);

        auto rampUp = std::make_shared<TweenAnimation>(0, 1, 4000, TweenType::Linear);
        r->Append(rampUp, "Up");

        auto rampDown = std::make_shared<TweenAnimation>(1, 0, 4000, TweenType::Linear);
        r->Append(rampDown, "Down");

        auto g = std::make_shared<ConstantAnimation>(0);
        auto b = std::make_shared<ConstantAnimation>(0);

        SetColorAnimation(r, g, b);
        SetOpacityAnimation(alphaAnimation);
        SetSpeedAnimation(speedAnimation);

        AddArea(CuboidArea::Front);
    }

}  // namespace huestream
