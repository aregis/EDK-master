/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#ifndef HUESTREAM_EFFECT_EFFECTS_BASE_COLORANIMATIONEFFECT_H_
#define HUESTREAM_EFFECT_EFFECTS_BASE_COLORANIMATIONEFFECT_H_

#include "huestream/effect/effects/base/AnimationEffect.h"

#include <string>

namespace huestream {

    /**
     abstract base class for an effect which has animations for color, opacity and intensity
     */
    class ColorAnimationEffect : public AnimationEffect {
    protected:
        /**
         constructor
         @see Effect()
         @param opacityBoundToIntensity Specifies whether the opacity should automatically follow the intensity
         */
        explicit ColorAnimationEffect(const std::string &name = "", unsigned int layer = 0, bool opacityBoundToIntensity = false);

    public:
    PROP_DEFINE(ColorAnimationEffect, AnimationPtr, r, R);
    PROP_DEFINE(ColorAnimationEffect, AnimationPtr, g, G);
    PROP_DEFINE(ColorAnimationEffect, AnimationPtr, b, B);
    PROP_DEFINE(ColorAnimationEffect, AnimationPtr, a, A);
    PROP_DEFINE(ColorAnimationEffect, AnimationPtr, i, I);
    PROP_DEFINE_BOOL(ColorAnimationEffect, bool, opacityBoundToIntensity, OpacityBoundToIntensity);


    public:
        AnimationListPtr GetAnimations() override;

        /**
         destructor
         */
        virtual ~ColorAnimationEffect() {}

        /**
         convenience method to set the animations for red green and blue color channels
         @param r Animation for red channel
         @param g Animation for green channel
         @param b Animation for blue channel
         */
        void SetColorAnimation(AnimationPtr r, AnimationPtr g, AnimationPtr b);

        /**
         convenience method to set the animation for opacity 
         @param a Animation for opacity (alpha)
         */
        void SetOpacityAnimation(AnimationPtr a);

        /**
         convenience method to set the animation for intensity
         @note if the intensity animation is not nullptr, it will override the implicit intensity set by the rgb animations
         @param i Animation for intensity (brightness)
         */
        void SetIntensityAnimation(AnimationPtr i);

        /**
         convenience method to set a fixed color
         @param color Color to set
         */
        void SetFixedColor(const Color &color);

        /**
         convenience method to set a fixed opacity
         @param opacity Opacity (alpha) to set
         */
        void SetFixedOpacity(double opacity);

        void Serialize(JSONNode *node) const override;

        void Deserialize(JSONNode *node) override;

    protected:
        void SetIntensity(Color *color) const;
    };

}  // namespace huestream

#endif  // HUESTREAM_EFFECT_EFFECTS_BASE_COLORANIMATIONEFFECT_H_
