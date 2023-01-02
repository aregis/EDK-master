/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/
/** @file */

#ifndef HUESTREAM_EFFECT_EFFECTS_LIGHTITERATOREFFECT_H_
#define HUESTREAM_EFFECT_EFFECTS_LIGHTITERATOREFFECT_H_

#include "huestream/config/Config.h"
#include "huestream/effect/effects/base/ColorAnimationEffect.h"

#include <string>
#include <map>
#include <memory>

namespace huestream {

    /**
     order in which lights can be iterated over
     */
    enum IterationOrder {
        IterationOrderLeftRight,    ///< from left to right
        IterationOrderFrontBack,    ///< from front to back
        IterationOrderTopBottom,    ///< from top to bottom
        IterationOrderClockwise,    ///< clockwise
        IterationOrderInOut,        ///< from close to center to further away from center
        IterationOrderRandom,       ///< random order
        IterationOrderGroup         ///< order in which the lights appear in the entertainment group
    };

    /**
     mode in which lights can be iterated over
     */
    enum IterationMode {
        IterationModeSingle,    ///< iterate over all lights once then stop
        IterationModeCycle,     ///< continuous and after the last light continue with the first light
        IterationModeBounce     ///< continuous and after the last light revert the order
    };

    /**
     effect which iterates an animations over individual lights with a certain time offset between them
     @note example application is creating so called chase lights
     */
    class LightIteratorEffect : public ColorAnimationEffect {
    public:
        static constexpr const char* type = "huestream.LightIteratorEffect";

        explicit LightIteratorEffect(std::string name = "",
                            unsigned int layer = 0,
                            IterationOrder order = IterationOrderGroup,
                            IterationMode mode = IterationModeCycle,
                            double offset = 0,
                            bool preamble = true,
                            bool invertOrder = false);


        virtual ~LightIteratorEffect();

        void UpdateGroup(GroupPtr group) override;

        AnimationListPtr GetAnimations() override;

        void InitializeAnimations() override;

        std::string GetTypeName() const override;

        Color GetColor(LightPtr light) override;

        void Serialize(JSONNode *node) const override;

        void Deserialize(JSONNode *node) override;

    /**
     set order in which lights are iterated over
     */
    PROP_DEFINE(LightIteratorEffect, IterationOrder, order, Order);

    /**
     set mode in which lights are iterated over
     */
    PROP_DEFINE(LightIteratorEffect, IterationMode, mode, Mode);

    /**
     set the offset in milliseconds when the animation on the next light starts after the previous light, may be longer or shorter than the animation duration
     */
    PROP_DEFINE(LightIteratorEffect, double, offset, Offset);

    PROP_DEFINE_BOOL(LightIteratorEffect, bool, preamble, Preamble);

    /**
     set whether the order is inverted from what is specified in the iteration order
     */
    PROP_DEFINE_BOOL(LightIteratorEffect, bool, invertOrder, InvertOrder);

    protected:
        void CreateAnimations(GroupPtr group);

        void RenderUpdate() override;

        void SerializeOrder(JSONNode *node) const;

        void SerializeMode(JSONNode *node) const;

        void DeserializeOrder(JSONNode *node);

        void DeserializeMode(JSONNode *node);

        bool CompareLights(LightPtr i, LightPtr j) const;

        static const std::map<IterationOrder, std::string> _orderSerializeMap;
        static const std::map<IterationMode, std::string> _modeSerializeMap;
        std::shared_ptr<std::map<std::string, AnimationListPtr>> _lightIdAnimationMap;
        GroupPtr _group;
    };
}  // namespace huestream

#endif  // HUESTREAM_EFFECT_EFFECTS_LIGHTITERATOREFFECT_H_
