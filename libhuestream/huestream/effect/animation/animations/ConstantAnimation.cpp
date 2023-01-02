/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <huestream/effect/animation/animations/ConstantAnimation.h>

#include <string>
#include <memory>

namespace huestream {

    PROP_IMPL(ConstantAnimation, double, length, Length);

    void ConstantAnimation::UpdateValue(double* /*value*/, double /*positionMs*/) {}

    ConstantAnimation::ConstantAnimation(double value, double length) : _length(length) { _value = value; }

    ConstantAnimation::ConstantAnimation(double value) : _length(INF) { _value = value; }

    ConstantAnimation::ConstantAnimation() : _length(INF) { _value = 0; }

    double ConstantAnimation::GetLengthMs() const { return _length; }

    AnimationPtr ConstantAnimation::Clone() {
        return std::make_shared<ConstantAnimation>(*this);
    }

    std::string ConstantAnimation::GetTypeName() const {
        return ConstantAnimation::type;
    }

    void ConstantAnimation::Serialize(JSONNode *node) const {
        Serializable::Serialize(node);
        SerializeValue(node, Animation::AttributeValue, _value);
        if (!IsEndless()) {
            SerializeValue(node, ConstantAnimation::AttributeLength, _length);
        }
    }

    void ConstantAnimation::Deserialize(JSONNode *node) {
        Serializable::Deserialize(node);
        DeserializeValue(node, ConstantAnimation::AttributeValue, &_value, 0);
        DeserializeValue(node, ConstantAnimation::AttributeLength, &_length, INF);
    }

}  // namespace huestream
