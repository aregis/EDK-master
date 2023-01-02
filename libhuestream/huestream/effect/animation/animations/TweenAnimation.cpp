/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <huestream/common/util/HueMath.h>
#include <huestream/effect/animation/animations/TweenAnimation.h>

#include <memory>
#include <string>

namespace huestream {

    static const bool default_at_beginning = true;
    static const bool default_value_present = true;
    static const int default_begin = 0;
    static const int default_end = 1;
    static const int default_time = 1000;
    static const TweenType default_tween_type = TweenType::Linear;

    PROP_IMPL(TweenAnimation, double, begin, Begin);
    PROP_IMPL(TweenAnimation, double, end, End);
    PROP_IMPL(TweenAnimation, double, time, Time);
    PROP_IMPL_BOOL(TweenAnimation, bool, beginValuePresent, BeginValuePresent);
    PROP_IMPL(TweenAnimation, TweenType, tweenType, TweenType);


    TweenAnimation::TweenAnimation(double valueBegin, double valueEnd, double timeMs, TweenType tweenType)
            : _begin(valueBegin), _end(valueEnd), _time(timeMs), _beginValuePresent(default_at_beginning),
              _tweenType(tweenType), at_beginning_(true) {}

    TweenAnimation::TweenAnimation(double valueEnd, double timeMs, TweenType tweenType)
            : _begin(default_begin), _end(valueEnd), _time(timeMs), _beginValuePresent(false),
              _tweenType(tweenType), at_beginning_(true) {}

    TweenAnimation::TweenAnimation() {
        InitializeDefault();
    }

    AnimationPtr TweenAnimation::Clone() {
        return std::make_shared<TweenAnimation>(*this);
    }

    void TweenAnimation::UpdateValue(double *value, double positionMs) {
        if (!_beginValuePresent && at_beginning_) {
            _begin = *value;
        }

        switch (_tweenType) {
            case TweenType::Linear: {
                *value = HueMath::linearTween(positionMs, _begin, _end, _time);
                break;
            }
            case TweenType::EaseInOutQuad: {
                *value = HueMath::easeInOutQuad(positionMs, _begin, _end, _time);
                break;
            }
            case TweenType::EaseInOutSine: {
                *value = HueMath::easeInOutSine(positionMs, _begin, _end, _time);
                break;
            }
        }

        if (*value == 0) {
            *value = 0;
        }

        at_beginning_ = false;
    }

    double TweenAnimation::GetPositionFromValue(double value) {
        double pos = 0;
        int value_rounded = static_cast<int>(value * 1000);
        do {
            switch (_tweenType) {
                case TweenType::Linear: {
                    int result_rounded = static_cast<int>(HueMath::linearTween(pos, _begin, _end, _time) * 1000);
                    if (result_rounded == value_rounded) {
                        return pos;
                    }
                    break;
                }
                case TweenType::EaseInOutQuad: {
                    int result_rounded = static_cast<int>(HueMath::easeInOutQuad(pos, _begin, _end, _time) * 1000);
                    if (result_rounded == value_rounded) {
                        return pos;
                    }
                    break;
                }
                case TweenType::EaseInOutSine: {
                    int result_rounded = static_cast<int>(HueMath::easeInOutSine(pos, _begin, _end, _time) * 1000);
                    if (result_rounded == value_rounded) {
                        return pos;
                    }
                    break;
                }
            }
            pos++;
        } while (pos <= _time);

        return Animation::GetPositionFromValue(value);
    }

    double TweenAnimation::GetLengthMs() const {
        return _time;
    }

    void TweenAnimation::Rewind() {
        Animation::Rewind();
        at_beginning_ = true;
    }

    std::string TweenAnimation::GetTypeName() const {
        return TweenAnimation::type;
    }

    void TweenAnimation::Serialize(JSONNode *node) const {
        Serializable::Serialize(node);
        AddTweenType(node, AttributeTweenType, _tweenType);

        if (_beginValuePresent)
            SerializeValue(node, AttributeBegin, _begin);

        SerializeValue(node, AttributeEnd, _end);
        SerializeValue(node, AttributeTime, _time);
    }

    void TweenAnimation::Deserialize(JSONNode *node) {
        Serializable::Deserialize(node);

        InitializeDefault();

        SetTweenTypeIfAttributeExists(node, AttributeTweenType, &_tweenType, default_tween_type);

        if (!SerializerHelper::IsAttributeSet(node, AttributeBegin)) {
            _beginValuePresent = false;
        } else {
            DeserializeValue(node, AttributeBeginValuePresent, &_beginValuePresent, true);
        }

        DeserializeValue(node, AttributeBegin, &_begin, default_begin);
        DeserializeValue(node, AttributeEnd, &_end, default_end);
        DeserializeValue(node, AttributeTime, &_time, default_time);
    }

    void TweenAnimation::InitializeDefault() {
        at_beginning_ = default_at_beginning;
        _beginValuePresent = default_value_present;
        _begin = default_begin;
        _end = default_end;
        _time = default_time;
        _tweenType = default_tween_type;
    }

}  // namespace huestream
