/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#ifndef HUESTREAM_EFFECT_EFFECTS_HITEFFECT_H_
#define HUESTREAM_EFFECT_EFFECTS_HITEFFECT_H_

#include "huestream/effect/effects/base/RadialEffect.h"

#include <string>

namespace huestream {

    class HitEffect : public RadialEffect {
    public:
        explicit HitEffect(const std::string &name = "", unsigned int layer = 0);

        virtual ~HitEffect() {}

        void PrepareEffect(const Color &color, double angle_degrees);

    protected:
        void RenderUpdate() override;

    private:
        void PrepareColor(const Color &color, double duration_ms);
    };

}  // namespace huestream

#endif  // HUESTREAM_EFFECT_EFFECTS_HITEFFECT_H_
