/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <huestream/effect/animation/animations/ConstantAnimation.h>
#include <huestream/effect/animation/animations/SequenceAnimation.h>
#include <huestream/effect/animation/animations/TweenAnimation.h>
#include <huestream_example_gui_win/BrightnessSquareWaveEffect.h>

#include <string>
#include <memory>

namespace huestream {

    BrightnessSquareWaveEffect::BrightnessSquareWaveEffect(const std::string &name, unsigned int layer)
            : LightSourceEffect(name, layer) {}

    void BrightnessSquareWaveEffect::PrepareEffect(std::shared_ptr<ConstantAnimation> speedAnimation,
                                                   std::shared_ptr<ConstantAnimation> alphaAnimation) {
        auto timeTransition = 1;
        auto timeSteady = 400;
        auto maxBrightness = 1;
        auto minBrightness = 0;

        auto g = std::make_shared<SequenceAnimation>(INF);
        auto r = std::make_shared<SequenceAnimation>(INF);
        auto b = std::make_shared<SequenceAnimation>(INF);

        //  Up
        auto rampUpGreena = std::make_shared<TweenAnimation>(minBrightness, maxBrightness, timeTransition,
                                                            TweenType::Linear);
        g->Append(rampUpGreena, "UpGreen");

        auto rampUpReda = std::make_shared<TweenAnimation>(minBrightness, maxBrightness, timeTransition,
                                                          TweenType::Linear);
        r->Append(rampUpReda, "UpRed");

        auto rampUpBluea = std::make_shared<TweenAnimation>(minBrightness, maxBrightness, timeTransition,
                                                           TweenType::Linear);
        b->Append(rampUpBluea, "UpRed");


        //  Steady on max
        auto steadyGreena = std::make_shared<TweenAnimation>(maxBrightness, maxBrightness, timeSteady,
                                                            TweenType::Linear);
        g->Append(steadyGreena, "steadyGreen");

        auto steadyReda = std::make_shared<TweenAnimation>(maxBrightness, maxBrightness, timeSteady, TweenType::Linear);
        r->Append(steadyReda, "steadyRed");

        auto steadyBluea = std::make_shared<TweenAnimation>(maxBrightness, maxBrightness, timeSteady, TweenType::Linear);
        b->Append(steadyBluea, "steadyBlue");


        //  Down
        auto rampDownGreena = std::make_shared<TweenAnimation>(maxBrightness, minBrightness, timeTransition,
                                                              TweenType::Linear);
        g->Append(rampDownGreena, "DownGreen");

        auto rampDownReda = std::make_shared<TweenAnimation>(maxBrightness, minBrightness, timeTransition,
                                                            TweenType::Linear);
        r->Append(rampDownReda, "DownRed");

        auto rampDownBluea = std::make_shared<TweenAnimation>(maxBrightness, minBrightness, timeTransition,
                                                             TweenType::Linear);
        b->Append(rampDownBluea, "DownBlue");


        //  Steady on min
        auto steadyGreenb = std::make_shared<TweenAnimation>(minBrightness, minBrightness, timeSteady,
                                                            TweenType::Linear);
        g->Append(steadyGreenb, "steadyGreen");

        auto steadyRedb = std::make_shared<TweenAnimation>(minBrightness, minBrightness, timeSteady, TweenType::Linear);
        r->Append(steadyRedb, "steadyRed");

        auto steadyBlueb = std::make_shared<TweenAnimation>(minBrightness, minBrightness, timeSteady, TweenType::Linear);
        b->Append(steadyBlueb, "steadyBlue");

        SetColorAnimation(r, g, b);
        SetOpacityAnimation(alphaAnimation);
        auto radius = std::make_shared<ConstantAnimation>(10);
        SetRadiusAnimation(radius);
        SetSpeedAnimation(speedAnimation);
    }
}  // namespace huestream
