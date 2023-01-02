/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <huestream/effect/animation/animations/ConstantAnimation.h>
#include <huestream/effect/effects/base/AnimationEffect.h>

#include <string>
#include <memory>

namespace huestream {

    PROP_IMPL(AnimationEffect, AnimationPtr, speed, Speed);

    void AnimationEffect::InitializeAnimations() {
        _animations = GetAnimations();
        _player->BindAnimations(_animations);

        AnimationListPtr speedAnimations = std::make_shared<AnimationList>();
        speedAnimations->push_back(_speed);
        _speedPlayer->BindAnimations(speedAnimations);
    }

    AnimationEffect::AnimationEffect(std::string name, unsigned int layer) :
        Effect(name, layer) {
        _speed.reset(new ConstantAnimation(1));
        _speedPlayer = std::make_shared<Player>();
        _player = std::make_shared<Player>();
    }

    void AnimationEffect::Enable() {
        Effect::Enable();
        InitializeAnimations();
        _player->Start();
        _speedPlayer->Start();
    }

    void AnimationEffect::Disable() {
        Effect::Disable();
        _player->Stop();
        _speedPlayer->Stop();
    }

    void AnimationEffect::Render() {
        _speedPlayer->UpdateMarkers();

        _player->SetSpeed(_speed->GetValue());

        _player->UpdateMarkers();
        if (_player->IsStopped()) {
            Finish();
        } else {
            RenderUpdate();
        }
    }

    void AnimationEffect::SetPlayer(PlayerPtr player) {
        _player = player;
    }

    double AnimationEffect::GetLength() {
        auto largestLength = 0.0;
        auto allAnimationsAreEndless = true;

        auto AnimationList = GetAnimations();
        for (auto animation : *AnimationList) {
            auto thisLength = animation->GetTotalLength();
            if (!animation->IsEndless() && thisLength > largestLength) {
                allAnimationsAreEndless = false;
                largestLength = thisLength;
            }
        }

        return allAnimationsAreEndless ? INF : largestLength;
    }

    bool AnimationEffect::IsEndless() {
        return GetLength() == INF;
    }

    void AnimationEffect::SetSpeedAnimation(AnimationPtr speed) {
        _speed = speed;
    }

    void AnimationEffect::Serialize(JSONNode *node) const {
        Effect::Serialize(node);

        SerializeAttribute(node, AttributeSpeed, _speed);
    }

    void AnimationEffect::Deserialize(JSONNode *node) {
        Effect::Deserialize(node);

        _speed = DeserializeAttribute<Animation>(node, AttributeSpeed, _speed);
    }

}  // namespace huestream
