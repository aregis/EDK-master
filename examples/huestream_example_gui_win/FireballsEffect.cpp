/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <huestream/config/Config.h>
#include <huestream/effect/animation/animations/ConstantAnimation.h>
#include <huestream/effect/animation/animations/SequenceAnimation.h>
#include <huestream/effect/animation/animations/TweenAnimation.h>
#include <huestream_example_gui_win/FireballsEffect.h>

#include <string>
#include <memory>

namespace huestream {

    FireballsEffect::FireballsEffect(const std::string &name, unsigned int layer) :
        LightSourceEffect(name, layer) {}

    void FireballsEffect::PrepareEffect(std::shared_ptr<ConstantAnimation> speedAnimation,
                                        std::shared_ptr<ConstantAnimation> alphaAnimation) {
        const auto time = 900;
        const auto radius = 0.9;
        const auto distanceFromCenter = 0.7;

        auto x = std::make_shared<SequenceAnimation>(INF);
        auto y = std::make_shared<SequenceAnimation>(INF);

        auto xLeft = std::make_shared<TweenAnimation>(-1 * distanceFromCenter, -1 * distanceFromCenter, time,
                                                     TweenType::Linear);
        auto xRight = std::make_shared<TweenAnimation>(distanceFromCenter, distanceFromCenter, time, TweenType::Linear);
        auto yFrontBack = std::make_shared<TweenAnimation>(1 + radius, -1 - radius, time, TweenType::Linear);
        auto yPause = std::make_shared<TweenAnimation>(1 + radius, 1 + radius, time, TweenType::Linear);

        x->Append(xLeft, "xLeft");
        y->Append(yFrontBack, "yFrontBack");

        x->Append(xRight, "xPauseRight");
        y->Append(yPause, "yPauseRight");

        x->Append(xRight, "xRight");
        y->Append(yFrontBack, "yFrontBack");

        x->Append(xLeft, "xPauseLeft");
        y->Append(yPause, "yPauseLeft");

        auto r = std::make_shared<ConstantAnimation>(1);
        auto g = std::make_shared<ConstantAnimation>(0.4);
        auto b = std::make_shared<ConstantAnimation>(0.1);

        SetPositionAnimation(x, y);
        SetColorAnimation(r, g, b);
        SetOpacityAnimation(alphaAnimation);
        SetRadiusAnimation(std::make_shared<ConstantAnimation>(radius));
        SetSpeedAnimation(speedAnimation);
    }

}  // namespace huestream
