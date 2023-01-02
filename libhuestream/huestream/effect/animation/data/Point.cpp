/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <huestream/effect/animation/data/Point.h>

#include <string>

namespace huestream {

    PROP_IMPL(Point, double, x, X);
    PROP_IMPL(Point, double, y, Y);

    std::string Point::GetTypeName() const {
        return type;
    }

    void Point::Serialize(JSONNode *node) const {
        Serializable::Serialize(node);
        SerializeValue(node, AttributeX, _x);
        SerializeValue(node, AttributeY, _y);
    }

    void Point::Deserialize(JSONNode *node) {
        Serializable::Deserialize(node);
        DeserializeValue(node, AttributeX, &_x, 0);
        DeserializeValue(node, AttributeY, &_y, 0);
    }


    Point::Point(double x, double y) : _x(x), _y(y) {}

    Point::Point() : _x(0), _y(0) {}

}  // namespace huestream
