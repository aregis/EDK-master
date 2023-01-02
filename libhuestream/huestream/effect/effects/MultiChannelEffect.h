/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#ifndef HUESTREAM_EFFECT_EFFECTS_MULTICHANNELEFFECT_H_
#define HUESTREAM_EFFECT_EFFECTS_MULTICHANNELEFFECT_H_

#include "huestream/effect/animation/data/Channel.h"
#include "huestream/common/data/Group.h"
#include "huestream/effect/effects/base/AnimationEffect.h"

#include <string>
#include <memory>
#include <map>

namespace huestream {

    /**
     effect which evenly distributes multiple channels over the available lights in a specific setup, while best effort matching channel and light locations
     @note especially useful when its more important that all animations are visible than that they are exactly at the right location (compare huestream::AreaEffect)
     */
    class MultiChannelEffect : public AnimationEffect {
    public:
        static constexpr const char* type = "huestream.MultiChannelEffect";

    PROP_DEFINE(MultiChannelEffect, ChannelListPtr, channels, Channels);

    public:
        explicit MultiChannelEffect(const std::string &name = "", unsigned int layer = 0);

        void OnUpdateChannels();

        virtual ~MultiChannelEffect() {}

        AnimationListPtr GetAnimations() override;

        void Enable() override;

        void UpdateGroup(GroupPtr group) override;

        Color GetColor(LightPtr light) override;

        void Clear();

        void AddChannel(ChannelPtr channel);

        std::string GetTypeName() const override;

        void Serialize(JSONNode *node) const override;

        void Deserialize(JSONNode *node) override;

    protected:
        GroupPtr _selectedGroup;

        std::unique_ptr<std::map<std::string, ChannelPtr>> _lightIdChannelMap;

        void RenderUpdate() override;

        void AssignLightsToChannels();

        static ChannelList::iterator FindChannelWithClosestLight(ChannelListPtr channels, LightListPtr lights);

        static LightList::iterator FindClosestLight(LightListPtr lights, Location position);
    };

}  // namespace huestream

#endif  // HUESTREAM_EFFECT_EFFECTS_MULTICHANNELEFFECT_H_
