/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#ifndef HUESTREAM_EFFECT_ANIMATION_ANIMATIONS_TWEENANIMATION_H_
#define HUESTREAM_EFFECT_ANIMATION_ANIMATIONS_TWEENANIMATION_H_

#include "huestream/effect/animation/animations/base/Animation.h"
#include "huestream/common/serialize/Serializable.h"
#include "huestream/effect/animation/data/TweenType.h"

#include <string>

namespace huestream {

    /**
     type of animation which goes from a certain start value to a certain end value in a certain time with a certain trajectory
     */
    class TweenAnimation : public Animation {
    public:
        static constexpr const char* type = "huestream.TweenAnimation";

        AnimationPtr Clone() override;

        std::string GetTypeName() const override;

        void Serialize(JSONNode *node) const override;

        void Deserialize(JSONNode *node) override;

    /**
     set begin value of the animation
     */
    PROP_DEFINE(TweenAnimation, double, begin, Begin);

    /**
     set end value of the animation
     */
    PROP_DEFINE(TweenAnimation, double, end, End);

    /**
     set duration of the animation in milliseconds
     */
    PROP_DEFINE(TweenAnimation, double, time, Time);

    PROP_DEFINE_BOOL(TweenAnimation, bool, beginValuePresent, BeginValuePresent);

    /**
     set the type of trajectory to go from begin to end value
     */
    PROP_DEFINE(TweenAnimation, TweenType, tweenType, TweenType);

    private:
        bool at_beginning_;

    public:
        /**
         constructor
         @param valueBegin Begin value of the animation
         @param valueEnd End value of the animation
         @param timeMs Duration in milliseconds of the animation
         @param type Type of trajectory to go from begin to end value
         */
        TweenAnimation(double valueBegin, double valueEnd, double timeMs, TweenType type);

        TweenAnimation(double valueEnd, double timeMs, TweenType type);

        TweenAnimation();

        /**
         destructor
         */
        virtual ~TweenAnimation() {}

        void UpdateValue(double *value, double positionMs) override;

        double GetPositionFromValue(double value) override;

        double GetLengthMs() const override;

        void Rewind() override;

        void InitializeDefault();
    };

}  // namespace huestream

#endif  // HUESTREAM_EFFECT_ANIMATION_ANIMATIONS_TWEENANIMATION_H_
