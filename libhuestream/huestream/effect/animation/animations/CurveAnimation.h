/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#ifndef HUESTREAM_EFFECT_ANIMATION_ANIMATIONS_CURVEANIMATION_H_
#define HUESTREAM_EFFECT_ANIMATION_ANIMATIONS_CURVEANIMATION_H_

#include "huestream/effect/animation/data/CurveData.h"
#include "huestream/effect/animation/animations/base/RepeatableAnimation.h"

#include <string>

namespace huestream {

    /**
     animation which describes a trajectory by a collection of time-value points
     */
    class CurveAnimation : public RepeatableAnimation {
    public:
        static constexpr const char* type = "huestream.CurveAnimation";
    PROP_DEFINE(CurveAnimation, CurveData, curveData, CurveData);

    public:
        AnimationPtr Clone() override;

        void Serialize(JSONNode *node) const override;

        void Deserialize(JSONNode *node) override;

        std::string GetTypeName() const override;

        void UpdateValue(double *value, double positionMs) override;

        double GetLength() const;

        void Append(const CurveData &other);

        CurveAnimation() {}

        virtual ~CurveAnimation() {}

        /**
         constructor
         @param points Points this curve consists of: x maps to position in milliseconds, y maps to value
         */
        explicit CurveAnimation(PointListPtr points, Nullable<CurveOptions> options = Nullable<CurveOptions>());

        /**
         constructor including repeatTimes
         @see RepeatableAnimation()
         @param points Points this curve consists of: x maps to position in milliseconds, y maps to value
         */
        CurveAnimation(double repeatTimes, PointListPtr points,
                      Nullable<CurveOptions> options = Nullable<CurveOptions>());

        double GetLengthMs() const override;

        /**
         append another curve to this curve
         */
        void Append(const CurveAnimation &other);

        /**
         append another point to this curve
         */
        void AppendPoint(const PointPtr point);

        /**
         append another curve to this curve with linearization enabled
         this means if the original last point varies less than delta from
         the linear interpolation between the original second to last point and new point
         the new point will replace the original last point
         */
        void AppendPointLinearized(const PointPtr point, double delta = 0.000015);

        const PointListPtr &GetPoints() const;
    };
}  // namespace huestream

#endif  // HUESTREAM_EFFECT_ANIMATION_ANIMATIONS_CURVEANIMATION_H_
