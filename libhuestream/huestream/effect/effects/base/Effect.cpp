/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <huestream/common/time/ITimeProvider.h>
#include <huestream/effect/effects/base/Effect.h>

#include <string>

namespace huestream {

    PROP_IMPL(Effect, std::string, name, Name);
    PROP_IMPL(Effect, unsigned int, layer, Layer);

    Effect::Effect(std::string name, unsigned int layer) :
        _name(name),
        _layer(layer),
        _state(State::Disabled) {
    }

    void Effect::Enable() {
        _state = State::Enabled;
    }

    void Effect::Disable() {
        _state = State::Disabled;
    }

    void Effect::UpdateGroup(GroupPtr /*group*/) {
    }

    bool Effect::IsFinished() const {
        return _state == State::Finished;
    }

    bool Effect::IsEnabled() const {
        return _state == State::Enabled;
    }

    bool Effect::IsDisabled() const {
        return _state == State::Disabled;
    }

    void Effect::Finish() {
        _state = State::Finished;
    }

    void Effect::Serialize(JSONNode *node) const {
        Serializable::Serialize(node);

        SerializeValue(node, AttributeName, _name);
    }

    void Effect::Deserialize(JSONNode *node) {
        Serializable::Deserialize(node);

        DeserializeValue(node, AttributeName, &_name, "");
    }

}  // namespace huestream
