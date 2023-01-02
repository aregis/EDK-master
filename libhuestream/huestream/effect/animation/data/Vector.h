/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#ifndef HUESTREAM_EFFECT_ANIMATION_DATA_VECTOR_H_
#define HUESTREAM_EFFECT_ANIMATION_DATA_VECTOR_H_

#include "huestream/effect/animation/data/Point.h"
#include "huestream/common/data/Location.h"

namespace huestream {

    class Vector {
    private:
        double _x;
        double _y;
        double _z;
    public:
        Vector(const double x, const double y, const double z = 0);

        Vector(const Point &from, const Point &to);

        Vector(const Location &from, const Location &to);

        const double &get_x() const;

        const double &get_y() const;

        const double &get_z() const;

        int GetQuadrant() const;

        double GetLength() const;

        double GetAngle() const;

        double GetAnglePhi() const;

        double GetAngleTheta() const;
    };
}  // namespace huestream

#endif  // HUESTREAM_EFFECT_ANIMATION_DATA_VECTOR_H_
