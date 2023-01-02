/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#ifndef HUESTREAM_EFFECT_ANIMATION_ANIMATIONS_BASE_REPEATABLEANIMATION_H_
#define HUESTREAM_EFFECT_ANIMATION_ANIMATIONS_BASE_REPEATABLEANIMATION_H_

#include "huestream/common/serialize/Serializable.h"
#include "huestream/effect/animation/animations/base/Animation.h"

namespace huestream {

    /**
     abstract base class for an animation that can be repeated a set number of times
     */
    class RepeatableAnimation : public Animation {
    public:
    /**
     set amount of times this animation repeats itself until it ends
     @note zero times repeating means it is played once
     */
    PROP_DEFINE(RepeatableAnimation, double, repeatTimes, RepeatTimes);

    public:
        /**
         constructor
         @param repeatTimes Amount of times this animation repeats itself until it ends
         @note zero times repeating means it is played once
         */
        explicit RepeatableAnimation(double repeatTimes);

        RepeatableAnimation();

        /**
         destructor
         */
        virtual ~RepeatableAnimation() {}

        void OnMarkerUpdate() override;

        void Serialize(JSONNode *node) const override;

        void Deserialize(JSONNode *node) override;

        bool IsEndless() const override;

        double GetTotalLength() const override;
    };

}  // namespace huestream

#endif  // HUESTREAM_EFFECT_ANIMATION_ANIMATIONS_BASE_REPEATABLEANIMATION_H_
