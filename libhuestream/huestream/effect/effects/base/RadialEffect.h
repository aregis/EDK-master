/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#ifndef HUESTREAM_EFFECT_EFFECTS_BASE_RADIALEFFECT_H_
#define HUESTREAM_EFFECT_EFFECTS_BASE_RADIALEFFECT_H_

#include "huestream/effect/animation/Player.h"
#include "huestream/effect/effects/base/Effect.h"
#include "huestream/effect/effects/base/ColorAnimationEffect.h"

#include <string>

namespace huestream {

    class RadialEffect : public ColorAnimationEffect {
    public:
        explicit RadialEffect(const std::string &name = "", unsigned int layer = 0);

        virtual ~RadialEffect() {}

        static constexpr const char* type = "huestream.RadialEffect";

        PROP_DEFINE(RadialEffect, double, angle, Angle);

    private:
        LightListPtr _effect_lights;
        GroupPtr _group;

    protected:
        void RenderUpdate() override;

        void MarkLights(double markPointDegrees);

    public:
        std::string GetTypeName() const override;

        void Serialize(JSONNode *node) const override;

        void Deserialize(JSONNode *node) override;

        void Enable() override;

        void UpdateGroup(GroupPtr group) override;

        Color GetColor(LightPtr light) override;
    };
}  // namespace huestream

#endif  // HUESTREAM_EFFECT_EFFECTS_BASE_RADIALEFFECT_H_
