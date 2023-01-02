/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <huestream/effect/animation/data/Vector.h>
#include <huestream/effect/animation/animations/ConstantAnimation.h>
#include <huestream/common/util/HueMath.h>
#include <huestream/effect/effects/LightSourceEffect.h>
#include <huestream/effect/animation/animations/base/AnimationHelper.h>

#include <iostream>
#include <string>
#include <vector>

namespace huestream {

    PROP_IMPL(LightSourceEffect, AnimationPtr, radius, Radius);
    PROP_IMPL(LightSourceEffect, AnimationPtr, x, X);
    PROP_IMPL(LightSourceEffect, AnimationPtr, y, Y);


    LightSourceEffect::LightSourceEffect(std::string name, unsigned int layer) :
        ColorAnimationEffect(name, layer, false) {
        _x.reset(new ConstantAnimation(0));
        _y.reset(new ConstantAnimation(0));
        _radius.reset(new ConstantAnimation(1));
    }

    void LightSourceEffect::SetPositionAnimation(AnimationPtr x, AnimationPtr y) {
        _x = x;
        _y = y;
    }

    void LightSourceEffect::SetRadiusAnimation(AnimationPtr radius) {
        _radius = radius;
    }

    void LightSourceEffect::RenderUpdate() {
    }

    Color LightSourceEffect::GetColor(LightPtr light) {
        auto radius = _radius->GetValue();
        auto PointRadius = Point(_x->GetValue(), _y->GetValue());
        auto PointLight = Point(light->GetPosition().GetX(), light->GetPosition().GetY());
        auto vector = Vector(PointRadius, PointLight);
        auto vecLen = vector.GetLength();
        auto currentAlpha = _a->GetValue();
        auto alphaAccent = (radius == 0) ? 0 : HueMath::easeInQuad(vecLen, 1, 0, radius) * currentAlpha;
        auto outputColor = Color(_r->GetValue(), _g->GetValue(), _b->GetValue(), alphaAccent);
        SetIntensity(&outputColor);
        return outputColor;
    }

    AnimationListPtr LightSourceEffect::GetAnimations() {
        auto list = ColorAnimationEffect::GetAnimations();
        list->push_back(_x);
        list->push_back(_y);
        list->push_back(_radius);
        return list;
    }

    std::string LightSourceEffect::GetTypeName() const {
        return type;
    }

    void LightSourceEffect::Serialize(JSONNode *node) const {
        ColorAnimationEffect::Serialize(node);

        SerializeAttribute(node, AttributeX, _x);
        SerializeAttribute(node, AttributeY, _y);
        SerializeAttribute(node, AttributeRadius, _radius);
    }

    void LightSourceEffect::Deserialize(JSONNode *node) {
        ColorAnimationEffect::Deserialize(node);

        _x = DeserializeAttribute<Animation>(node, AttributeX, _x);
        _y = DeserializeAttribute<Animation>(node, AttributeY, _y);
        _radius = DeserializeAttribute<Animation>(node, AttributeRadius, _radius);
    }

    typedef LightSourceEffect CircleLightSourceEffect;

}  // namespace huestream
