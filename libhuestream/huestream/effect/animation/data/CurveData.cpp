/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include "huestream/common/util/HueMath.h"
#include "huestream/effect/animation/data/CurveData.h"

#include <assert.h>

#include <iostream>
#include <string>
#include <algorithm>
#include <cmath>

namespace huestream {

PROP_IMPL(CurveData, Nullable<CurveOptions>, options, Options);
PROP_IMPL(CurveData, PointListPtr, points, Points);

CurveData::CurveData() :
    _options(Nullable<CurveOptions>()),
    _points(NEW_LIST_PTR(Point)),
    _lastIndex(0) {
}

CurveData::CurveData(PointListPtr points, Nullable<CurveOptions> options) :
    _options(options),
    _points(NEW_LIST_PTR(Point)),
    _lastIndex(0) {
    if (points != nullptr) {
        for (auto point : *points) {
            _points->push_back(point);
        }
    }
}

double CurveData::CorrectValue(double v) const {
    if (!_options.has_value()) {
        return v;
    }

    auto factor = _options.get_value().GetMultiplyFactor();
    auto result = v * factor;

    if (_options.get_value().GetClipMin().has_value()) {
        auto clip = _options.get_value().GetClipMin().get_value();
        if (result < clip)
            result = clip;
    }

    if (_options.get_value().GetClipMax().has_value()) {
        auto clip = _options.get_value().GetClipMax().get_value();
        if (result > clip)
            result = clip;
    }

    return result;
}

double CurveData::GetLength() const {
    if (!HasPoints())
        return 0;

    return GetEnd() - GetBegin() + 1;
}

double CurveData::GetBegin() const {
    if (!HasPoints()) return 0;
    return _points->front()->GetX();
}

double CurveData::GetEnd() const {
    if (!HasPoints()) return 0;
    return _points->back()->GetX();
}

bool CurveData::HasPoints() const {
    return _points->size() > 0;
}

bool CurveData::IsClosestLower(PointList::iterator it, double x) {
    auto isValidAndLower = (it != _points->end()) && ((*it)->GetX() <= x);
    auto isLastOrNextIsHigher = ((it + 1) == _points->end()) || ((*(it + 1))->GetX() > x);
    return isValidAndLower && isLastOrNextIsHigher;
}

PointList::iterator CurveData::GetStartPoint(double x) {
    if (_lastIndex >= _points->size())
        _lastIndex = 0;

    //shortcut last or next point, as it applies in most cases
    auto lastIter = _points->begin() + _lastIndex;
    for (auto it = lastIter; it < lastIter + 2; ++it) {
        if (IsClosestLower(it, x)) {
            _lastIndex = it - _points->begin();
            return it;
        }
    }

    //else perform binary search
    auto result = std::upper_bound(_points->begin(), _points->end(), x,
        [](const double x, const PointPtr &p) { return x < p->GetX(); });

    if (result == _points->begin()) {
        result = _points->end();
    } else {
        result--;
        _lastIndex = result - _points->begin();
    }

    return result;
}

bool CurveData::GetStartAndEndPoint(double x, PointPtr *start, PointPtr *end) {
    auto iter = GetStartPoint(x);
    
    if (iter == _points->end() || (iter + 1) == _points->end())
        return false;
    
    *start = *iter;
    *end = *(iter + 1);
    return true;
}

double CurveData::Interpolate(PointPtr start, PointPtr end, double x) {
    auto time = end->GetX() - start->GetX();
    auto absX = x - start->GetX();
    auto beginValue = start->GetY();
    auto endValue = end->GetY();
    return HueMath::linearTween(absX, beginValue, endValue, time);
}

double CurveData::GetInterpolatedValue(double x) {
    if (_points->size() < 2) return _points->front()->GetY();
    if (x <= GetBegin()) return _points->front()->GetY();
    if (x >= GetEnd()) return _points->back()->GetY();

    PointPtr start;
    PointPtr end;
    GetStartAndEndPoint(x, &start, &end);

    auto interpolatedValue = Interpolate(start, end, x);

    auto corrected = CorrectValue(interpolatedValue);

    return corrected;
}

PointPtr CurveData::GetInterpolated(double x) {
    return std::make_shared<Point>(x, GetInterpolatedValue(x));
}

double CurveData::GetStepValue(double x) {
    if (_points->size() < 2) return _points->front()->GetY();
    if (x <= GetBegin()) return _points->front()->GetY();
    if (x >= GetEnd()) return _points->back()->GetY();

    PointPtr start;
    PointPtr end;

    GetStartAndEndPoint(x, &start, &end);
    return start->GetY();
}

const CurveData &CurveData::Append(const CurveData &other) const {
    auto offsetX = 0.0;

    if (_points->size()) {
        offsetX = _points->back()->GetX();
    }

    for (auto point : *other._points) {
        _points->push_back(NEW_PTR(Point, point->GetX() + offsetX, point->GetY()));
    }

    return *this;
}

void CurveData::AppendPoint(const PointPtr point) {
    _points->push_back(point);
}

void CurveData::AppendPointLinearized(const PointPtr point, double delta) {
    if (_points->size() >= 2) {
        auto last       = (*_points)[_points->size() - 1];
        auto secondLast = (*_points)[_points->size() - 2];
        auto y_inter = Interpolate(secondLast, point, last->GetX());
        auto y_orig = last->GetY();
        if (std::abs(y_orig - y_inter) <= delta) {
            _points->pop_back();
        }
    }
    _points->push_back(point);
}

std::string CurveData::GetTypeName() const {
    return type;
}

void CurveData::Serialize(JSONNode *node) const {
    Serializable::Serialize(node);

    if (_options.has_value()) {
        JSONNode v(JSON_NODE);
        _options.get_value().Serialize(&v);
        v.set_name(AttributeOptions);
        node->push_back(v);
    }

    JSONNode arrayNode(JSON_ARRAY);
    for (size_t i = 0; i < _points->size(); ++i) {
        JSONNode v;
        auto p = _points->at(i);
        p->Serialize(&v);
        arrayNode.push_back(v);
    }
    arrayNode.set_name(AttributePoints);
    node->push_back(arrayNode);
}

void CurveData::Deserialize(JSONNode *node) {
    Serializable::Deserialize(node);

    _options.clear_value();
    if (SerializerHelper::IsAttributeSet(node, AttributeOptions)) {
        auto j = (*node)[AttributeOptions];
        auto o = CurveOptions();
        o.Deserialize(&j);
        _options.set_value(o);
    }

    _points->clear();
    if (SerializerHelper::IsAttributeSet(node, AttributePoints)) {
        auto j = (*node)[AttributePoints];
        for (auto pointIt = j.begin(); pointIt != j.end(); ++pointIt) {
            auto pointJ = *pointIt;
            auto p = NEW_PTR(Point);
            p->Deserialize(&pointJ);
            _points->push_back(p);
        }
    }
}

}  // namespace huestream
