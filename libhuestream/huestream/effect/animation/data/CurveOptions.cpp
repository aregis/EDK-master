/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <huestream/effect/animation/data/CurveOptions.h>

#include <string>

namespace huestream {

    PROP_IMPL(CurveOptions, double, multiplyFactor, MultiplyFactor);
    PROP_IMPL(CurveOptions, Nullable<double>, clipMin, ClipMin);
    PROP_IMPL(CurveOptions, Nullable<double>, clipMax, ClipMax);


    CurveOptions::CurveOptions(double multiplyFactor, const Nullable<double> clipMin, const Nullable<double> clipMax)
            : _multiplyFactor(multiplyFactor), _clipMin(clipMin), _clipMax(clipMax) {
    }

    std::string CurveOptions::GetTypeName() const {
        return type;
    }

    void CurveOptions::Serialize(JSONNode *node) const {
        Serializable::Serialize(node);
        SerializeValue(node, AttributeMultiplyFactor, _multiplyFactor);
        if (_clipMin.has_value())
            SerializeValue(node, AttributeClipMin, _clipMin.get_value());
        if (_clipMax.has_value())
            SerializeValue(node, AttributeClipMax, _clipMax.get_value());
    }

    void CurveOptions::Deserialize(JSONNode *node) {
        Serializable::Deserialize(node);
        DeserializeValue(node, AttributeMultiplyFactor, &_multiplyFactor, 1);

        if (SerializerHelper::IsAttributeSet(node, AttributeClipMin)) {
            _clipMin.set_value((*node)[AttributeClipMin].as_float());
        } else {
            _clipMin.clear_value();
        }
        if (SerializerHelper::IsAttributeSet(node, AttributeClipMax)) {
            _clipMax.set_value((*node)[AttributeClipMax].as_float());
        } else {
            _clipMax.clear_value();
        }
    }
}  // namespace huestream
