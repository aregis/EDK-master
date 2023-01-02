/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <huestream/effect/animation/animations/ConstantAnimation.h>
#include <huestream/effect/animation/animations/TweenAnimation.h>
#include <huestream/effect/effects/HitEffect.h>

#include <string>
#include <vector>
#include <memory>

namespace huestream {

    HitEffect::HitEffect(const std::string &name, unsigned int layer) :
        RadialEffect(name, layer) {}

    void HitEffect::RenderUpdate() {}

    void HitEffect::PrepareEffect(const Color &color, double angle_degrees) {
        PrepareColor(color, 500);
        SetAngle(angle_degrees);
    }

    void HitEffect::PrepareColor(const Color &color, double duration_ms) {
        std::vector<std::shared_ptr<Animation>> colorAnimations;

        for (auto &current_color : color.GetRGBA()) {
            std::shared_ptr<TweenAnimation> currAnimation(
                    new TweenAnimation(current_color, 0, duration_ms, TweenType::Linear));
            colorAnimations.push_back(currAnimation);
        }

        SetColorAnimation(colorAnimations[0], colorAnimations[1], colorAnimations[2]);
        SetOpacityAnimation(colorAnimations[3]);
    }

}  // namespace huestream
