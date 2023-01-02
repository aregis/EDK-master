/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <huestream/common/util/HueMath.h>

#include <algorithm>
#include <utility>

namespace HueMath {

    double linearTween(double t, double b, double e, double d) {
        double c = e - b;
        if (t > d) return e;

        return c * t / d + b;
    }

    double easeInQuad(double t, double b, double e, double d) {
        double c = e - b;
        if (t > d) return e;

        t /= d;
        return c * t * t + b;
    }

    double easeOutQuad(double t, double b, double e, double d) {
        double c = e - b;
        if (t > d) return e;

        t /= d;
        return -c * t * (t - 2) + b;
    }

    double easeInOutQuad(double t, double b, double e, double d) {
        double c = e - b;
        if (t > d) return e;

        t /= d / 2;
        if (t < 1) return c / 2 * t * t + b;
        t--;
        return -c / 2 * (t * (t - 2) - 1) + b;
    }

    double easeInCubic(double t, double b, double e, double d) {
        double c = e - b;
        if (t > d) return e;

        t /= d;
        return c * t * t * t + b;
    }

    double easeOutCubic(double t, double b, double e, double d) {
        double c = e - b;
        if (t > d) return e;

        t /= d;
        t--;
        return c * (t * t * t + 1) + b;
    }

    double easeInOutCubic(double t, double b, double e, double d) {
        double c = e - b;
        if (t > d) return e;

        t /= d / 2;
        if (t < 1) return c / 2 * t * t * t + b;
        t -= 2;
        return c / 2 * (t * t * t + 2) + b;
    }

    double easeInQuart(double t, double b, double e, double d) {
        double c = e - b;
        if (t > d) return e;

        t /= d;
        return c * t * t * t * t + b;
    }

    double easeOutQuart(double t, double b, double e, double d) {
        double c = e - b;
        if (t > d) return e;

        t /= d;
        t--;
        return -c * (t * t * t * t - 1) + b;
    }

    double easeInOutQuart(double t, double b, double e, double d) {
        double c = e - b;
        if (t > d) return e;

        t /= d / 2;
        if (t < 1) return c / 2 * t * t * t * t + b;
        t -= 2;
        return -c / 2 * (t * t * t * t - 2) + b;
    }

    double easeInQuint(double t, double b, double e, double d) {
        double c = e - b;
        if (t > d) return e;

        t /= d;
        return c * t * t * t * t * t + b;
    }

    double easeOutQuint(double t, double b, double e, double d) {
        double c = e - b;
        if (t > d) return e;

        t /= d;
        t--;
        return c * (t * t * t * t * t + 1) + b;
    }

    double easeInOutQuint(double t, double b, double e, double d) {
        double c = e - b;
        if (t > d) return e;

        t /= d / 2;
        if (t < 1) return c / 2 * t * t * t * t * t + b;
        t -= 2;
        return c / 2 * (t * t * t * t * t + 2) + b;
    }

    double easeInSine(double t, double b, double e, double d) {
        double c = e - b;
        if (t > d) return e;

        return -c * cos(t / d * (pi() / 2)) + c + b;
    }

    double easeOutSine(double t, double b, double e, double d) {
        double c = e - b;
        if (t > d) return e;

        return c * sin(t / d * (pi() / 2)) + b;
    }

    double easeInOutSine(double t, double b, double e, double d) {
        double c = e - b;
        if (t > d) return e;

        return -c / 2 * (cos(pi() * t / d) - 1) + b;
    }

    double easeInExpo(double t, double b, double e, double d) {
        double c = e - b;
        if (t > d) return e;

        return c * pow(2, 10 * (t / d - 1)) + b;
    }

    double easeOutExpo(double t, double b, double e, double d) {
        double c = e - b;
        if (t > d) return e;

        return c * (-pow(2, -10 * t / d) + 1) + b;
    }

    double easeInOutExpo(double t, double b, double e, double d) {
        double c = e - b;
        if (t > d) return e;

        t /= d / 2;
        if (t < 1) return c / 2 * pow(2, 10 * (t - 1)) + b;
        t--;
        return c / 2 * (-pow(2, -10 * t) + 2) + b;
    }

    double easeInCirc(double t, double b, double e, double d) {
        double c = e - b;
        if (t > d) return e;

        t /= d;
        return -c * (sqrt(1 - t * t) - 1) + b;
    }

    double easeOutCirc(double t, double b, double e, double d) {
        double c = e - b;
        if (t > d) return e;

        t /= d;
        t--;
        return c * sqrt(1 - t * t) + b;
    }

    double easeInOutCirc(double t, double b, double e, double d) {
        double c = e - b;
        if (t > d) return e;

        t /= d / 2;
        if (t < 1) return -c / 2 * (sqrt(1 - t * t) - 1) + b;
        t -= 2;
        return c / 2 * (sqrt(1 - t * t) + 1) + b;
    }

    bool isAngleBetween(double target, double angle1, double angle2) {
        // make the angle from angle1 to angle2 to be <= 180 degrees
        double rAngle = std::fmod(std::fmod((angle2 - angle1), 360) + 360, 360);
        if (rAngle >= 180)
            std::swap(angle1, angle2);

        // check if it passes through zero
        if (angle1 <= angle2)
            return target >= angle1 && target <= angle2;

        return target >= angle1 || target <= angle2;
    }

    double getDiffDegrees(double a, double b) {
        double minDegrees = std::min(b - a, 360 + a - b);

        if (minDegrees < 0) {
            return minDegrees * -1;
        }

        return minDegrees;
    }

}  // namespace HueMath
