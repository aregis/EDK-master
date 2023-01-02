/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <huestream/effect/effects/MultiChannelEffect.h>
#include <huestream/effect/animation/data/Vector.h>
#include <huestream/effect/Mixer.h>

#include <memory>
#include <string>
#include <map>
#include <utility>

using std::unique_ptr;
using std::make_shared;

namespace huestream {

PROP_IMPL_ON_UPDATE_CALL(MultiChannelEffect, ChannelListPtr, channels, Channels, OnUpdateChannels);


MultiChannelEffect::MultiChannelEffect(const std::string &name, unsigned int layer)
    : AnimationEffect(name, layer),
      _channels(std::make_shared<ChannelList>()),
      _selectedGroup(std::make_shared<Group>()),
      _lightIdChannelMap(unique_ptr<std::map<std::string, ChannelPtr>>(new std::map<std::string, ChannelPtr>)) {
}

void MultiChannelEffect::OnUpdateChannels() {
    AssignLightsToChannels();
}

void MultiChannelEffect::AddChannel(ChannelPtr channel) {
    _channels->push_back(channel);
    AssignLightsToChannels();
}

void MultiChannelEffect::Clear() {
    _channels->clear();
    _lightIdChannelMap->clear();
}

AnimationListPtr MultiChannelEffect::GetAnimations() {
    auto animations = std::make_shared<AnimationList>();
    for (auto channel : *_channels) {
        animations->push_back(channel->GetR());
        animations->push_back(channel->GetG());
        animations->push_back(channel->GetB());
        animations->push_back(channel->GetA());
    }
    return animations;
}

void MultiChannelEffect::Enable() {
    AssignLightsToChannels();
    AnimationEffect::Enable();
}

void MultiChannelEffect::UpdateGroup(GroupPtr group) {
    _selectedGroup = group;
    AssignLightsToChannels();
}

Color MultiChannelEffect::GetColor(LightPtr light) {
    auto it = _lightIdChannelMap->find(light->GetId());
    if (it != _lightIdChannelMap->end()) {
        auto c = it->second;
        return Color(c->GetR()->GetValue(), c->GetG()->GetValue(), c->GetB()->GetValue(), c->GetA()->GetValue());
    }
    return Color();
}

std::string MultiChannelEffect::GetTypeName() const {
    return type;
}

void MultiChannelEffect::Serialize(JSONNode *node) const {
    AnimationEffect::Serialize(node);

    SerializeList(node, AttributeChannels, _channels);
}

void MultiChannelEffect::Deserialize(JSONNode *node) {
    AnimationEffect::Deserialize(node);

    DeserializeList<ChannelListPtr, Channel>(node, &_channels, AttributeChannels);

    AssignLightsToChannels();
}

void MultiChannelEffect::RenderUpdate() {
}

void MultiChannelEffect::AssignLightsToChannels() {
    _lightIdChannelMap->clear();

    if (_channels->empty())
        return;

    auto lights = _selectedGroup->GetLights();
    auto lightsCopy = make_shared<LightList>(*lights);

    while (!lightsCopy->empty()) {
        auto channelsCopy = make_shared<ChannelList>(*_channels);

        while (!channelsCopy->empty() && !lightsCopy->empty()) {
            auto channelIt = FindChannelWithClosestLight(channelsCopy, lightsCopy);
            auto lightIt = FindClosestLight(lightsCopy, (*channelIt)->GetPosition());

            _lightIdChannelMap->insert(std::pair<std::string, ChannelPtr>((*lightIt)->GetId(), *channelIt));

            channelsCopy->erase(channelIt);
            lightsCopy->erase(lightIt);
        }
    }
}

ChannelList::iterator
MultiChannelEffect::FindChannelWithClosestLight(ChannelListPtr channels, LightListPtr lights) {
    auto ClosestChannelIt = channels->begin();
    auto smallestDistance = INF;

    for (auto channelIt = channels->begin(); channelIt != channels->end(); ++channelIt) {
        auto lightIt = FindClosestLight(lights, (*channelIt)->GetPosition());
        auto distance = Vector((*channelIt)->GetPosition(), (*lightIt)->GetPosition()).GetLength();
        if (distance < smallestDistance) {
            ClosestChannelIt = channelIt;
            smallestDistance = distance;
        }
    }

    return ClosestChannelIt;
}

LightList::iterator MultiChannelEffect::FindClosestLight(LightListPtr lights, Location position) {
    auto closestLightIt = lights->begin();
    auto smallestDistance = INF;

    for (auto lightIt = lights->begin(); lightIt != lights->end(); ++lightIt) {
        auto distance = Vector(position, (*lightIt)->GetPosition()).GetLength();
        if (distance < smallestDistance) {
            closestLightIt = lightIt;
            smallestDistance = distance;
        }
    }

    return closestLightIt;
}

}  // namespace huestream
