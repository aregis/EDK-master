/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#ifndef HUESTREAM_EFFECT_ANIMATION_ANIMATIONS_RANDOMANIMATION_H_
#define HUESTREAM_EFFECT_ANIMATION_ANIMATIONS_RANDOMANIMATION_H_

#include "huestream/effect/animation/animations/TweenAnimation.h"

#include <string>

namespace huestream {

    /**
     type of animation which generates transitions of bound random length to bound random values
     */
    class RandomAnimation : public Animation {
    public:
        static constexpr const char* type = "huestream.Rand";

    /**
     set bound for the minimum value the animation could generate
     */
    PROP_DEFINE(RandomAnimation, double, minValue, MinValue);

    /**
     set bound for the maximum value the animation could generate
     */
    PROP_DEFINE(RandomAnimation, double, maxValue, MaxValue);

    /**
     set bound for minimum length of a transition between two values
     */
    PROP_DEFINE(RandomAnimation, double, minInterval, MinInterval);

    /**
     set bound for maximum length of a transition between two values
     */
    PROP_DEFINE(RandomAnimation, double, maxInterval, MaxInterval);

    /**
     set type of trajectory for transition between two values
     */
    PROP_DEFINE(RandomAnimation, TweenType, tweenType, TweenType);

    /**
     set length of the total animation, can be INF
     */
    PROP_DEFINE(RandomAnimation, double, length, Length);

    public:
        RandomAnimation();

        /**
         constructor
         @param min_value Minimum value the animation can generate
         @param max_value Maximum value the animation can generate
         @param min_interval Minimum length of a transition between two values
         @param max_interval Maximum length of a transition between two values
         @param type Type of trajectory for transition between two values
         @param length Total length of the animation, default infinite
         */
        RandomAnimation(double min_value,
                           double max_value,
                           double min_interval,
                           double max_interval,
                           TweenType type,
                           double length = INF);

        /**
         destructor
         */
        virtual ~RandomAnimation() {}

        AnimationPtr Clone() override;

        std::string GetTypeName() const override;

        void UpdateValue(double *value, double positionMs) override;

        double GetLengthMs() const override;

        void Serialize(JSONNode *node) const override;

        void Deserialize(JSONNode *node) override;

    protected:
        double offset_;
        TweenAnimation *_tweenAnimation;
    };

}  // namespace huestream

#endif  // HUESTREAM_EFFECT_ANIMATION_ANIMATIONS_RANDOMANIMATION_H_
