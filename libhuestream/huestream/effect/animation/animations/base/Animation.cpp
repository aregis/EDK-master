/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <huestream/effect/animation/animations/base/Animation.h>

#include <string>

namespace huestream {

    Animation::Animation() : _marker(0), _value(0) {}

    PROP_IMPL_ON_UPDATE_CALL(Animation, double, marker, Marker, OnMarkerUpdate);
    PROP_IMPL(Animation, double, value, Value);

    void Animation::OnMarkerUpdate() {
        UpdateValue(&_value, _marker);
    }


    void Animation::Rewind() {
        SetMarker(0);
    }

    bool Animation::IsEndless() const {
        return GetTotalLength() == INF;
    }

    bool Animation::IsPlaying() const {
        return _marker < GetTotalLength();
    }

    double Animation::GetTotalLength() const {
        return GetLengthMs();
    }

    std::string Animation::GetTypeName() const {
        return "";
    }

    void Animation::Serialize(JSONNode *node) const {
        Serializable::Serialize(node);
    }

    void Animation::Deserialize(JSONNode *node) {
        Serializable::Deserialize(node);
    }

    double Animation::GetPositionFromValue(double /*value*/) {
        return 0;
    }

    void Animation::SetTweenTypeIfAttributeExists(const JSONNode *node, const std::string &attributeName, TweenType *value,
                                                  TweenType default_value) {
        std::string tweenType;
        DeserializeValue(node, attributeName, &tweenType, TweenTypeHelper::ToString(default_value));
        *value = TweenTypeHelper::FromString(tweenType);
    }

    void Animation::AddTweenType(JSONNode *node, const std::string &attribute_name, const TweenType &value) {
        SerializeValue(node, attribute_name, TweenTypeHelper::ToString(value));
    }


}  // namespace huestream
