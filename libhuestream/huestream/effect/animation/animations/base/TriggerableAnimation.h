/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#ifndef HUESTREAM_EFFECT_ANIMATION_ANIMATIONS_BASE_TRIGGERABLEANIMATION_H_
#define HUESTREAM_EFFECT_ANIMATION_ANIMATIONS_BASE_TRIGGERABLEANIMATION_H_

#include "huestream/effect/animation/animations/base/RepeatableAnimation.h"
#include <string>

namespace huestream {

class TriggerableAnimation : public RepeatableAnimation {
 public:
    explicit TriggerableAnimation(double repeatTimes);

    TriggerableAnimation();

    virtual void Trigger(const std::string& bookmark) = 0;

    virtual void TriggerOnLevel(const std::string& bookmark) = 0;

    virtual double GetOffset() const;

 protected:
    double offset_;
};

}  // namespace huestream

#endif  // HUESTREAM_EFFECT_ANIMATION_ANIMATIONS_BASE_TRIGGERABLEANIMATION_H_
