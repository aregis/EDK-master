/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#ifndef HUESTREAM_EFFECT_ANIMATION_DATA_CURVEDATA_H_
#define HUESTREAM_EFFECT_ANIMATION_DATA_CURVEDATA_H_

#include "huestream/effect/animation/data/Point.h"
#include "huestream/effect/animation/data/CurveOptions.h"

#include <vector>
#include <memory>
#include <string>

namespace huestream {

    class CurveData : public virtual Serializable {
    public:
        static constexpr const char* type = "huestream.CurveData";

        PROP_DEFINE(CurveData, Nullable<CurveOptions>, options, Options);
        PROP_DEFINE(CurveData, PointListPtr, points, Points);

    public:
        void Serialize(JSONNode *node) const override;

        void Deserialize(JSONNode *node) override;

        std::string GetTypeName() const override;

        CurveData();

        virtual ~CurveData() {}

        explicit CurveData(PointListPtr points, Nullable<CurveOptions> options = Nullable<CurveOptions>());

        PointPtr GetInterpolated(double x);

        double GetInterpolatedValue(double x);

        double GetStepValue(double x);

        double GetLength() const;

        double GetBegin() const;

        double GetEnd() const;

        bool HasPoints() const;

        const CurveData &Append(const CurveData &other) const;

        void AppendPoint(const PointPtr Point);

        void AppendPointLinearized(const PointPtr Point, double delta = 0.000015);

    private:
        double CorrectValue(double v) const;

        bool GetStartAndEndPoint(double x, PointPtr* start, PointPtr* end);

        PointList::iterator GetStartPoint(double x);

        bool IsClosestLower(PointList::iterator it, double x);

        double Interpolate(PointPtr start, PointPtr end, double x);

        unsigned int _lastIndex;
    };
}  // namespace huestream

#endif  // HUESTREAM_EFFECT_ANIMATION_DATA_CURVEDATA_H_
