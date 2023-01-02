/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <huestream/common/data/Light.h>

#include <iostream>
#include <string>

namespace huestream {

Light::Light() : Light("", Location(0, 0)) {
}

Light::Light(std::string id, Location pos, std::string name, std::string model, std::string archetype, bool reachable, bool on)
    : _id(id), _idV1(id), _name(name), _model(model), _archetype(archetype), _color(), _position(pos), _reachable(reachable), _on(on), _dynamic(false), _dynamicEnabled(false), _brightness(0.0) {
}

Light::~Light() {
}

PROP_IMPL(Light, std::string, id, Id);
PROP_IMPL(Light, std::string, idV1, IdV1);
PROP_IMPL(Light, std::string, name, Name);
PROP_IMPL(Light, std::string, model, Model);
PROP_IMPL(Light, Color, color, Color);
PROP_IMPL(Light, Location, position, Position);
PROP_IMPL_BOOL(Light, bool, reachable, Reachable);
PROP_IMPL_BOOL(Light, bool, on, On);
PROP_IMPL(Light, double, brightness, Brightness);
PROP_IMPL(Light, std::string, archetype, Archetype);
PROP_IMPL_BOOL(Light, bool, dynamic, Dynamic);
PROP_IMPL_BOOL(Light, bool, dynamicEnabled, DynamicEnabled);

void Light::Serialize(JSONNode *node) const {
    Serializable::Serialize(node);
    SerializeValue(node, AttributeId, _id);
    SerializeValue(node, AttributeIdV1, _idV1);
    SerializeValue(node, AttributeName, _name);
    SerializeValue(node, AttributeModel, _model);
    SerializeValue(node, AttributeArchetype, _archetype);
    SerializeValue(node, AttributeDynamic, _dynamic);
    SerializeMember(node, AttributeColor, _color);
    SerializeMember(node, AttributePosition, _position);
}

void Light::Deserialize(JSONNode *node) {
    Serializable::Deserialize(node);
    DeserializeValue(node, AttributeId, &_id, "");
    DeserializeValue(node, AttributeIdV1, &_idV1, "");
    DeserializeValue(node, AttributeName, &_name, "");
    DeserializeValue(node, AttributeModel, &_model, "");
    DeserializeValue(node, AttributeArchetype, &_archetype, "");
    DeserializeValue(node, AttributeDynamic, &_dynamic, false);
    SetMemberIfAttributeExists(node, AttributeColor, &_color);
    SetMemberIfAttributeExists(node, AttributePosition, &_position);
}

Light* Light::Clone() {
    auto l = new Light();
    l->_id = _id;
    l->_idV1 = _idV1;
    l->_name = _name;
    l->_model = _model;
    l->_archetype = _archetype;
    l->_color = _color;
    l->_position = _position;
    l->_reachable = _reachable;
    l->_on = _on;
    l->_brightness = _brightness;
    l->_dynamic = _dynamic;
    l->_dynamicEnabled = _dynamicEnabled;
    return  l;
}

std::string Light::GetTypeName() const {
    return type;
}

}  // namespace huestream
