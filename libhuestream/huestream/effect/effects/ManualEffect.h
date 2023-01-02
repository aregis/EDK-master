/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#ifndef HUESTREAM_EFFECT_EFFECTS_MANUALEFFECT_H_
#define HUESTREAM_EFFECT_EFFECTS_MANUALEFFECT_H_

#include "huestream/effect/effects/base/Effect.h"

#include <string>
#include <map>

namespace huestream {

    class ManualEffect : public Effect {
    public:
        static constexpr const char* type = "huestream.ManualEffect";

        std::string GetTypeName() const override;

        explicit ManualEffect(std::string name = "", unsigned int layer = 0);

        virtual ~ManualEffect();

        void SetIdToColor(std::string id, const Color &color);

        Color GetColor(LightPtr light) override;

        void Render() override;
    protected:
        std::map<std::string, Color> _idColorMap;
    };
}  // namespace huestream

#endif  // HUESTREAM_EFFECT_EFFECTS_MANUALEFFECT_H_
