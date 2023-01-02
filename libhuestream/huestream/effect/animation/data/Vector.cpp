/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <huestream/effect/animation/data/Vector.h>

#include <cmath>

#define PI 3.14159265

namespace huestream {

    Vector::Vector(const double x, const double y, const double z) : _x(x), _y(y), _z(z) {}

    Vector::Vector(const Point &from, const Point &to) : _x(to.GetX() - from.GetX()),
                                                         _y(to.GetY() - from.GetY()),
                                                         _z(0) {}

    Vector::Vector(const Location &from, const Location &to) : _x(to.GetX() - from.GetX()),
                                                               _y(to.GetY() - from.GetY()),
                                                               _z(to.GetZ() - from.GetZ()) {}

    double Vector::GetLength() const {
        return std::sqrt(_x * _x + _y * _y + _z * _z);
    }

    int Vector::GetQuadrant() const {
        if (_x >= 0) {
            if (_y >= 0) return 0;
            return 3;
        }
        if (_y >= 0) return 1;
        return 2;
    }

//                90
//                |
//        Q1     ^|     Q0
//               y|
//  180 ----------|---------- 0
//                |  x >
//        Q2      |     Q3
//                |
//               270

    double Vector::GetAngle() const {
        return GetAnglePhi();
    }

    double Vector::GetAnglePhi() const {
        auto angle = std::atan2(_y, _x) * 180 / PI;

        if (angle < 0)
            angle += 360;

        return angle;
    }

    double Vector::GetAngleTheta() const {
        auto angle = std::acos(_z / GetLength()) * 180 / PI;

        if (angle < 0)
            angle += 360;

        return angle;
    }

    const double &Vector::get_y() const {
        return _y;
    }

    const double &Vector::get_x() const {
        return _x;
    }

    const double &Vector::get_z() const {
        return _z;
    }

}  // namespace huestream
