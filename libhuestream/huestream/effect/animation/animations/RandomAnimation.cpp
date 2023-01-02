/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <huestream/common/util/Rand.h>
#include <huestream/effect/animation/animations/RandomAnimation.h>

#include <string>
#include <memory>

namespace huestream {

    PROP_IMPL(RandomAnimation, double, minValue, MinValue);
    PROP_IMPL(RandomAnimation, double, maxValue, MaxValue);
    PROP_IMPL(RandomAnimation, double, minInterval, MinInterval);
    PROP_IMPL(RandomAnimation, double, maxInterval, MaxInterval);
    PROP_IMPL(RandomAnimation, TweenType, tweenType, TweenType);
    PROP_IMPL(RandomAnimation, double, length, Length);

    static const int default_min_value = 0;
    static const int default_max_value = 1;
    static const int default_min_interval = 500;
    static const int default_max_interval = 1000;
    static const TweenType default_tween_type = TweenType::Linear;
    static const double default_length = INF;

    AnimationPtr RandomAnimation::Clone() {
        return std::make_shared<RandomAnimation>(*this);
    }

    void RandomAnimation::UpdateValue(double *value, double positionMs) {
        auto relativePos = positionMs - offset_;
        if (_tweenAnimation != nullptr && relativePos >= _tweenAnimation->GetTotalLength()) {
            delete _tweenAnimation;
            _tweenAnimation = nullptr;
        }

        if (_tweenAnimation == nullptr) {
            offset_ = positionMs;
            relativePos = positionMs - offset_;
            auto timeLeft = _length - positionMs;
            auto randVal = Rand(_minValue, _maxValue);
            auto randInterval = Rand(_minInterval, _maxInterval);

            if (randInterval > timeLeft) randInterval = timeLeft;
            _tweenAnimation = new TweenAnimation(randVal, randInterval, _tweenType);
        }

        _tweenAnimation->SetValue(*value);
        _tweenAnimation->SetMarker(relativePos);
        *value = _tweenAnimation->GetValue();
    }

    double RandomAnimation::GetLengthMs() const {
        return _length;
    }

    RandomAnimation::RandomAnimation(double min_value,
                                           double max_value,
                                           double min_interval,
                                           double max_interval,
                                           TweenType tweenType,
                                           double length) : _minValue(min_value),
                                                            _maxValue(max_value),
                                                            _minInterval(min_interval),
                                                            _maxInterval(max_interval),
                                                            _tweenType(tweenType),
                                                            _length(length), offset_(0),
                                                            _tweenAnimation(nullptr) {}

    RandomAnimation::RandomAnimation() : _minValue(default_min_value),
                                               _maxValue(default_max_value),
                                               _minInterval(default_min_interval),
                                               _maxInterval(default_max_interval),
                                               _tweenType(default_tween_type),
                                               _length(default_length), offset_(0),
                                               _tweenAnimation(nullptr) {}


    std::string RandomAnimation::GetTypeName() const {
        return type;
    }

    void RandomAnimation::Serialize(JSONNode *node) const {
        Serializable::Serialize(node);
        SerializeValue(node, AttributeMinValue, _minValue);
        SerializeValue(node, AttributeMaxValue, _maxValue);
        SerializeValue(node, AttributeMinInterval, _minInterval);
        SerializeValue(node, AttributeMaxInterval, _maxInterval);
        AddTweenType(node, AttributeTweenType, _tweenType);
        if (_length != INF)
            SerializeValue(node, AttributeLength, _length);
    }

    void RandomAnimation::Deserialize(JSONNode *node) {
        Serializable::Deserialize(node);
        DeserializeValue(node, AttributeMinValue, &_minValue, default_min_value);
        DeserializeValue(node, AttributeMaxValue, &_maxValue, default_max_value);
        DeserializeValue(node, AttributeMinInterval, &_minInterval, default_min_interval);
        DeserializeValue(node, AttributeMaxInterval, &_maxInterval, default_max_interval);
        SetTweenTypeIfAttributeExists(node, AttributeTweenType, &_tweenType, default_tween_type);
        DeserializeValue(node, AttributeLength, &_length, default_length);
    }


}  // namespace huestream
