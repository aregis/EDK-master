/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <huestream/common/data/Location.h>
#include <huestream/effect/animation/data/Channel.h>
#include <huestream/effect/animation/animations/ConstantAnimation.h>

#include <string>
#include <memory>

namespace huestream {

PROP_IMPL(Channel, Location, position, Position);
PROP_IMPL(Channel, AnimationPtr, r, R);
PROP_IMPL(Channel, AnimationPtr, g, G);
PROP_IMPL(Channel, AnimationPtr, b, B);
PROP_IMPL(Channel, AnimationPtr, a, A);

std::string Channel::GetTypeName() const {
    return type;
}

void Channel::Serialize(JSONNode *node) const {
    Serializable::Serialize(node);
    SerializeAttribute(node, AttributePosition, std::make_shared<Location>(_position));
    SerializeAttribute(node, AttributeR, _r);
    SerializeAttribute(node, AttributeG, _g);
    SerializeAttribute(node, AttributeB, _b);
    SerializeAttribute(node, AttributeA, _a);
}

void Channel::Deserialize(JSONNode *node) {
    Serializable::Deserialize(node);
    _position = *DeserializeAttribute<Location>(node, AttributePosition, std::make_shared<Location>(_position));
    _r = DeserializeAttribute<Animation>(node, AttributeR, _r);
    _g = DeserializeAttribute<Animation>(node, AttributeG, _g);
    _b = DeserializeAttribute<Animation>(node, AttributeB, _b);
    _a = DeserializeAttribute<Animation>(node, AttributeA, _a);
}

Channel::Channel(Location pos, AnimationPtr r, AnimationPtr g, AnimationPtr b, AnimationPtr a) {
    _position = pos;
    _r = r;
    _g = g;
    _b = b;
    _a = a;
}

Channel::Channel() {
    _position = Location(0, 0);
    _r = std::make_shared<ConstantAnimation>(1);
    _g = std::make_shared<ConstantAnimation>(1);
    _b = std::make_shared<ConstantAnimation>(1);
    _a = std::make_shared<ConstantAnimation>(1);
}

}  // namespace huestream
