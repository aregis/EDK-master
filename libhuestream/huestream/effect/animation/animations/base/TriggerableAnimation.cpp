/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <huestream/effect/animation/animations/base/TriggerableAnimation.h>

namespace huestream {

        TriggerableAnimation::TriggerableAnimation(double repeatTimes): RepeatableAnimation(repeatTimes), offset_(0) {}

        TriggerableAnimation::TriggerableAnimation() : RepeatableAnimation(true), offset_(0) {}

        double TriggerableAnimation::GetOffset() const {
            return offset_;
        }

}  // namespace huestream
