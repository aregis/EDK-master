/*******************************************************************************
 Copyright (C) 2018 Philips Lighting Holding B.V.
 All Rights Reserved.
 ********************************************************************************/

#include <huestream/effect/animation/data/Vector.h>
#include <huestream/effect/animation/animations/ConstantAnimation.h>
#include <huestream/common/util/HueMath.h>
#include <huestream/effect/effects/SphereLightSourceEffect.h>
#include <huestream/effect/animation/animations/base/AnimationHelper.h>

#include <iostream>
#include <string>
#include <vector>

namespace huestream {

    PROP_IMPL(SphereLightSourceEffect, AnimationPtr, radius, Radius);
    PROP_IMPL(SphereLightSourceEffect, AnimationPtr, x, X);
    PROP_IMPL(SphereLightSourceEffect, AnimationPtr, y, Y);
    PROP_IMPL(SphereLightSourceEffect, AnimationPtr, z, Z);

    SphereLightSourceEffect::SphereLightSourceEffect(std::string name, unsigned int layer) :
        ColorAnimationEffect(name, layer, false) {
        _x.reset(new ConstantAnimation(0));
        _y.reset(new ConstantAnimation(0));
        _z.reset(new ConstantAnimation(0));
        _radius.reset(new ConstantAnimation(1));
    }

    void SphereLightSourceEffect::SetPositionAnimation(AnimationPtr x, AnimationPtr y, AnimationPtr z) {
        _x = x;
        _y = y;
        _z = z;
    }

    void SphereLightSourceEffect::SetRadiusAnimation(AnimationPtr radius) {
        _radius = radius;
    }

    void SphereLightSourceEffect::RenderUpdate() {
    }

    Color SphereLightSourceEffect::GetColor(LightPtr light) {
        auto sourcePosition = Location(_x->GetValue(), _y->GetValue(), _z->GetValue());
        auto lightToSourceDistance = (Vector(sourcePosition, light->GetPosition())).GetLength();
        auto radius = _radius->GetValue();
        auto alphaAccent = (radius == 0) ? 0 : HueMath::easeInQuad(lightToSourceDistance, 1, 0, radius) * _a->GetValue();
        auto outputColor = Color(_r->GetValue(), _g->GetValue(), _b->GetValue(), alphaAccent);
        SetIntensity(&outputColor);
        return outputColor;
    }

    AnimationListPtr SphereLightSourceEffect::GetAnimations() {
        auto list = ColorAnimationEffect::GetAnimations();
        list->push_back(_x);
        list->push_back(_y);
        list->push_back(_z);
        list->push_back(_radius);
        return list;
    }

    std::string SphereLightSourceEffect::GetTypeName() const {
        return type;
    }

    void SphereLightSourceEffect::Serialize(JSONNode *node) const {
        ColorAnimationEffect::Serialize(node);

        SerializeAttribute(node, AttributeX, _x);
        SerializeAttribute(node, AttributeY, _y);
        SerializeAttribute(node, AttributeZ, _z);
        SerializeAttribute(node, AttributeRadius, _radius);
    }

    void SphereLightSourceEffect::Deserialize(JSONNode *node) {
        ColorAnimationEffect::Deserialize(node);

        _x = DeserializeAttribute<Animation>(node, AttributeX, _x);
        _y = DeserializeAttribute<Animation>(node, AttributeY, _y);
        _z = DeserializeAttribute<Animation>(node, AttributeZ, _z);
        _radius = DeserializeAttribute<Animation>(node, AttributeRadius, _radius);
    }

}  // namespace huestream
