/*******************************************************************************
 Copyright (C) 2018 Philips Lighting Holding B.V.
 All Rights Reserved.
 ********************************************************************************/

#ifndef HUESTREAM_EFFECT_EFFECTS_SPHERELIGHTSOURCEEFFECT_H_
#define HUESTREAM_EFFECT_EFFECTS_SPHERELIGHTSOURCEEFFECT_H_

#include "huestream/common/data/Color.h"
#include "huestream/effect/animation/animations/base/Animation.h"
#include "huestream/effect/animation/data/CurveData.h"
#include "huestream/effect/effects/base/ColorAnimationEffect.h"

#include <string>

namespace huestream {

    /**
     effect which maps a virtual light source with a position and radius animation to actual lights in an entertainment setup
     */
    class SphereLightSourceEffect : public ColorAnimationEffect {
    public:
        static constexpr const char* type = "huestream.SphereLightSourceEffect";

    PROP_DEFINE(SphereLightSourceEffect, AnimationPtr, radius, Radius);
    PROP_DEFINE(SphereLightSourceEffect, AnimationPtr, x, X);
    PROP_DEFINE(SphereLightSourceEffect, AnimationPtr, y, Y);
    PROP_DEFINE(SphereLightSourceEffect, AnimationPtr, z, Z);

    public:
        std::string GetTypeName() const override;

        void Serialize(JSONNode *node) const override;

        void Deserialize(JSONNode *node) override;

        /**
         constructor
         @see Effect()
         */
        explicit SphereLightSourceEffect(std::string name = "", unsigned int layer = 0);

        /**
         destructor
         */
        virtual ~SphereLightSourceEffect() {}

        /**
         convenience method to set the animations for the x, y and z position of the virtual light source
         @param x Animation for x coordinate
         @param y Animation for y coordinate
         @param z Animation for z coordinate
         */
        void SetPositionAnimation(AnimationPtr x, AnimationPtr y, AnimationPtr z);

        /**
         convenience method to set the animation for the beam radius of the virtual light source
         @param radius Animation for radius
         */
        void SetRadiusAnimation(AnimationPtr radius);

        Color GetColor(LightPtr light) override;

        void RenderUpdate() override;

        AnimationListPtr GetAnimations() override;
    };
}  // namespace huestream

#endif  // HUESTREAM_EFFECT_EFFECTS_SPHERELIGHTSOURCEEFFECT_H_
