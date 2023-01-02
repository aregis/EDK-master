/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#ifndef HUESTREAM_EFFECT_ANIMATION_ANIMATIONS_CONSTANTANIMATION_H_
#define HUESTREAM_EFFECT_ANIMATION_ANIMATIONS_CONSTANTANIMATION_H_

#include "huestream/effect/animation/animations/base/Animation.h"
#include "huestream/common/serialize/Serializable.h"

#include <memory>
#include <string>

namespace huestream {

/**
 type of animation which always returns the value it was set to
 */
class ConstantAnimation : public Animation {
 public:
    static constexpr const char* type = "huestream.ConstantAnimation";
 PROP_DEFINE(ConstantAnimation, double, length, Length);

 public:
    ConstantAnimation();
    virtual ~ConstantAnimation() {}

    void Serialize(JSONNode *node) const override;

    void Deserialize(JSONNode *node) override;

    AnimationPtr Clone() override;

    std::string GetTypeName() const override;

    double GetLengthMs() const override;

    void UpdateValue(double *value, double positionMs) override;

    ConstantAnimation(double value, double length);

    /**
     constructor
     @param value The value this animation will be at any time
     */
    explicit ConstantAnimation(double value);
};
}  // namespace huestream

#endif  // HUESTREAM_EFFECT_ANIMATION_ANIMATIONS_CONSTANTANIMATION_H_
