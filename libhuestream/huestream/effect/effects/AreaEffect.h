/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#ifndef HUESTREAM_EFFECT_EFFECTS_AREAEFFECT_H_
#define HUESTREAM_EFFECT_EFFECTS_AREAEFFECT_H_

#include "huestream/config/Config.h"
#include "huestream/common/data/IArea.h"
#include "huestream/common/data/Area.h"
#include "huestream/effect/effects/base/ColorAnimationEffect.h"

#include <string>

namespace huestream {

    /**
     effect which is shown on all lights in a certain area
     @note especially useful when it only makes sense to shown an animation in a certain area (compare huestream::MultiChannelEffect)
     */
    class AreaEffect : public ColorAnimationEffect {
    public:
        static constexpr const char* type = "huestream.AreaEffect";

    PROP_DEFINE(AreaEffect, AreaListPtr, areas, Areas);

    public:
        std::string GetTypeName() const override;

        void Serialize(JSONNode *node) const override;

        void Deserialize(JSONNode *node) override;

        Color GetColor(LightPtr light) override;

    protected:
        void RenderUpdate() override;

    public:
        /**
         constructor
         @see ColorAnimationEffect()
         */
        explicit AreaEffect(std::string name = "", unsigned int layer = 0, bool isAlphaBoundToBrightness = false);

        /**
         destructor
         */
        virtual ~AreaEffect();

        /**
         add area to list of areas in which this effect will be rendered
         @param area Additional area
         */
        virtual void AddArea(const IArea &area);

        /**
         set a single area in which this effect will be rendered
         @param area New area
         */
        virtual void SetArea(const IArea &area);
    };
}  // namespace huestream

#endif  // HUESTREAM_EFFECT_EFFECTS_AREAEFFECT_H_
