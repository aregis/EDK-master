/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <huestream/effect/effects/base/AnimationEffect.h>
#include <huestream/effect/lightscript/Action.h>
#include <huestream/effect/animation/ActionPlayer.h>
#include <huestream/effect/effects/AreaEffect.h>

#include <string>
#include <memory>
#include <limits>

namespace huestream {

    PROP_IMPL(Action, int64_t, startPosition, StartPosition);
    PROP_IMPL(Action, int64_t, endPosition, EndPosition);
    PROP_IMPL_ON_UPDATE_CALL(Action, AnimationEffectPtr, effect, Effect, OnUpdateEffect);

    Action::Action() : Action("", 0, nullptr, 0, -1) {}

    Action::Action(std::string name, unsigned int layer, AnimationEffectPtr effect, int64_t startPosition, int64_t endPosition) :
            Effect(name, layer),
            _startPosition(startPosition),
            _endPosition(endPosition) {
        _actionplayer = std::make_shared<ActionPlayer>();
        SetEffect(effect);
        _state = State::Enabled;
    }

    void Action::SetTimeProvider(TimeProviderPtr timeProvider) {
        _timeProvider = timeProvider;
    }

    void Action::OnUpdateEffect() {
        if (_effect) {
            _effect->SetPlayer(_actionplayer);
            _effect->Enable();
        }
    }

    int64_t Action::GetCalculatedEndPosition() const {
        if (_effect == nullptr)
            return 0;

        auto effectLength = _effect->GetLength();
        auto endPos = std::numeric_limits<int64_t>::max();

        if (effectLength == INF) {
            if (_endPosition > 0) {
                endPos = _endPosition;
            }
        } else if (_endPosition < 0 || (effectLength < static_cast<double>(_endPosition - _startPosition))) {
            endPos = _startPosition + static_cast<int64_t>(effectLength);
        } else {
            endPos = _endPosition;
        }

        return endPos;
    }

    Color Action::GetColor(LightPtr light) {
        return _effect->GetColor(light);
    }

    void Action::UpdateGroup(GroupPtr group) {
        if (_effect != nullptr) {
            _effect->UpdateGroup(group);
        }
    }

    void Action::Render() {
        if (_timeProvider == nullptr || _effect == nullptr)
            return;

        _actionplayer->SetPosition(_timeProvider->Now() - _startPosition);
        _effect->Render();
    }

    bool Action::IsEnabled() const {
        if (_timeProvider == nullptr || _effect == nullptr)
            return false;

        return _effect->IsEnabled() && IsBetweenStartAndEnd(_timeProvider->Now());
    }

    std::string Action::GetTypeName() const {
        return type;
    }

    void Action::Serialize(JSONNode *node) const {
        Effect::Serialize(node);

        SerializeValue(node, AttributeStartPosition, _startPosition);

        if (_endPosition >= 0)
            SerializeValue(node, AttributeEndPosition, _endPosition);

        SerializeAttribute(node, AttributeEffect, _effect);
    }

    void Action::Deserialize(JSONNode *node) {
        Effect::Deserialize(node);

        int value;

        DeserializeValue(node, AttributeStartPosition, &value, 0);
        _startPosition = static_cast<int64_t>(value);
        DeserializeValue(node, AttributeEndPosition, &value, -1);
        _endPosition = static_cast<int64_t>(value);

        auto effect = DeserializeAttribute<AnimationEffect>(node, AttributeEffect, _effect);
        SetEffect(effect);
    }

    bool Action::IsBetweenStartAndEnd(int64_t position) const {
        return (position > _startPosition && position < GetCalculatedEndPosition());
    }
}  // namespace huestream
