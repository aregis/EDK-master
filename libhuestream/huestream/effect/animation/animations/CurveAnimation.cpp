/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <huestream/effect/animation/animations/CurveAnimation.h>

#include <iostream>
#include <string>
#include <memory>

namespace huestream {

    PROP_IMPL(CurveAnimation, CurveData, curveData, CurveData);

    AnimationPtr CurveAnimation::Clone() {
        return std::make_shared<CurveAnimation>(*this);
    }

    void CurveAnimation::Serialize(JSONNode *node) const {
        RepeatableAnimation::Serialize(node);
        JSONNode v;
        _curveData.Serialize(&v);
        v.set_name(AttributeCurveData);
        node->push_back(v);
    }

    void CurveAnimation::Deserialize(JSONNode *node) {
        RepeatableAnimation::Deserialize(node);
        if (SerializerHelper::IsAttributeSet(node, AttributeCurveData)) {
            JSONNode v = (*node)[AttributeCurveData];
            _curveData.Deserialize(&v);
        }
    }

    std::string CurveAnimation::GetTypeName() const {
        return type;
    }

    void CurveAnimation::UpdateValue(double *value, double positionMs) {
        *value = _curveData.GetInterpolatedValue(positionMs);
    }

    double CurveAnimation::GetLengthMs() const {
        return _curveData.GetLength();
    }

    CurveAnimation::CurveAnimation(PointListPtr points, Nullable<CurveOptions> options)
            : RepeatableAnimation(0), _curveData(points, options) {}

    CurveAnimation::CurveAnimation(double repeatTimes, PointListPtr points, Nullable<CurveOptions> options)
            : RepeatableAnimation(repeatTimes), _curveData(points, options) {}


    double CurveAnimation::GetLength() const {
        return _curveData.GetLength();
    }

    void CurveAnimation::Append(const CurveAnimation &other) {
        _curveData.Append(other._curveData);
    }

    void CurveAnimation::Append(const CurveData &other) {
        _curveData.Append(other);
    }

    void CurveAnimation::AppendPoint(const PointPtr point) {
        _curveData.AppendPoint(point);
    }

    void CurveAnimation::AppendPointLinearized(const PointPtr point, double delta) {
        _curveData.AppendPointLinearized(point, delta);
    }

    const PointListPtr &CurveAnimation::GetPoints() const {
        return _curveData.GetPoints();
    }
}  // namespace huestream
