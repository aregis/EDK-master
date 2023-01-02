/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <huestream/common/data/Location.h>

#include <string>

namespace huestream {

PROP_IMPL(Location, double, x, X);
PROP_IMPL(Location, double, y, Y);
PROP_IMPL(Location, double, z, Z);

Location::Location(double x, double y, double z)
    : _x(x), _y(y), _z(z) {
}

Location::Location(double x, double y)
    : _x(x), _y(y), _z(0) {
}

Location::Location()
    : _x(0), _y(0), _z(0) {
}

Location::~Location() {
}

std::string Location::GetTypeName() const {
    return type;
}

void Location::Serialize(JSONNode *node) const {
    Serializable::Serialize(node);
    SerializeValue(node, AttributeX, _x);
    SerializeValue(node, AttributeY, _y);
    SerializeValue(node, AttributeZ, _z);
}

void Location::Deserialize(JSONNode *node) {
    Serializable::Deserialize(node);
    DeserializeValue(node, AttributeX, &_x, 0);
    DeserializeValue(node, AttributeY, &_y, 0);
    DeserializeValue(node, AttributeZ, &_z, 0);
}

}  // namespace huestream
