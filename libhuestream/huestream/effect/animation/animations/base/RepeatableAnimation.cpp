/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <huestream/effect/animation/animations/base/RepeatableAnimation.h>

namespace huestream {

    PROP_IMPL(RepeatableAnimation, double, repeatTimes, RepeatTimes);

    RepeatableAnimation::RepeatableAnimation(double repeatTimes) : _repeatTimes(repeatTimes) {}

    RepeatableAnimation::RepeatableAnimation() : _repeatTimes(0) {}

    void RepeatableAnimation::OnMarkerUpdate() {
        auto times = (int64_t) (_marker / GetLengthMs());

        if (times <= _repeatTimes) {
            auto correction = times * GetLengthMs();
            _marker -= correction;
        }

        UpdateValue(&_value, _marker);
    }

    bool RepeatableAnimation::IsEndless() const {
        return GetTotalLength() == INF;
    }

    double RepeatableAnimation::GetTotalLength() const {
        if (GetLengthMs() == INF || _repeatTimes == INF) return INF;
        return GetLengthMs() * (_repeatTimes + 1);
    }

    void RepeatableAnimation::Serialize(JSONNode *node) const {
        Animation::Serialize(node);
        if (!IsEndless())
            SerializeValue(node, AttributeRepeatTimes, _repeatTimes);
    }

    void RepeatableAnimation::Deserialize(JSONNode *node) {
        Animation::Deserialize(node);
        DeserializeValue(node, AttributeRepeatTimes, &_repeatTimes, INF);
    }
}  // namespace huestream
