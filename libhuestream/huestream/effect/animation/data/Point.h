/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/
/** @file */

#ifndef HUESTREAM_EFFECT_ANIMATION_DATA_POINT_H_
#define HUESTREAM_EFFECT_ANIMATION_DATA_POINT_H_

#include "huestream/common/serialize/Serializable.h"

#include <stdarg.h>

#include <string>
#include <vector>

namespace huestream {

    /**
     convenience class for generic point with x and y value
     */
    class Point : public Serializable {
    public:
        std::string GetTypeName() const override;

        void Serialize(JSONNode *node) const override;

        void Deserialize(JSONNode *node) override;

        static constexpr const char* type = "huestream.Point";

    PROP_DEFINE(Point, double, x, X);
    PROP_DEFINE(Point, double, y, Y);

    public:
        Point(double _x, double _y);

        Point();
    };

    /**
     shared pointer to a huestream::Point object
     */
    SMART_POINTER_TYPES_FOR(Point);

}  // namespace huestream

#endif  // HUESTREAM_EFFECT_ANIMATION_DATA_POINT_H_
