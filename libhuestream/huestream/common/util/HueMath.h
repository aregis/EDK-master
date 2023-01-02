/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#ifndef HUESTREAM_COMMON_UTIL_HUEMATH_H_
#define HUESTREAM_COMMON_UTIL_HUEMATH_H_

#include <cmath>

// for more info: http://gizma.com/easing/#quad3
// for easing cheatsheet: http://easings.net/nl

namespace HueMath {

    inline double pi() { return std::acos(-1); }

    double linearTween(double t, double b, double e, double d);

    double easeInQuad(double t, double b, double e, double d);

    double easeOutQuad(double t, double b, double e, double d);

    double easeInOutQuad(double t, double b, double e, double d);

    double easeInCubic(double t, double b, double e, double d);

    double easeOutCubic(double t, double b, double e, double d);

    double easeInOutCubic(double t, double b, double e, double d);

    double easeInQuart(double t, double b, double e, double d);

    double easeOutQuart(double t, double b, double e, double d);

    double easeInOutQuart(double t, double b, double e, double d);

    double easeInQuint(double t, double b, double e, double d);

    double easeOutQuint(double t, double b, double e, double d);

    double easeInOutQuint(double t, double b, double e, double d);

    double easeInSine(double t, double b, double e, double d);

    double easeOutSine(double t, double b, double e, double d);

    double easeInOutSine(double t, double b, double e, double d);

    double easeInExpo(double t, double b, double e, double d);

    double easeOutExpo(double t, double b, double e, double d);

    double easeInOutExpo(double t, double b, double e, double d);

    double easeInCirc(double t, double b, double e, double d);

    double easeOutCirc(double t, double b, double c, double d);

    double easeInOutCirc(double t, double b, double c, double d);

    bool isAngleBetween(double target, double angle1, double angle2);

    double getDiffDegrees(double a, double b);
}  // namespace HueMath

#endif  // HUESTREAM_COMMON_UTIL_HUEMATH_H_
