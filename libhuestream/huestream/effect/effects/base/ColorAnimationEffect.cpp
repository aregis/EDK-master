/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <huestream/effect/animation/animations/ConstantAnimation.h>
#include <huestream/effect/effects/base/ColorAnimationEffect.h>
#include <huestream/effect/animation/animations/base/AnimationHelper.h>

#include <string>
#include <memory>

namespace huestream {

    PROP_IMPL(ColorAnimationEffect, AnimationPtr, r, R);
    PROP_IMPL(ColorAnimationEffect, AnimationPtr, g, G);
    PROP_IMPL(ColorAnimationEffect, AnimationPtr, b, B);
    PROP_IMPL(ColorAnimationEffect, AnimationPtr, a, A);
    PROP_IMPL(ColorAnimationEffect, AnimationPtr, i, I);
    PROP_IMPL_BOOL(ColorAnimationEffect, bool, opacityBoundToIntensity, OpacityBoundToIntensity);

    ColorAnimationEffect::ColorAnimationEffect(const std::string &name, unsigned int layer, bool opacityBoundToIntensity) :
        AnimationEffect(name, layer) {
        _r.reset(new ConstantAnimation(1));
        _g.reset(new ConstantAnimation(1));
        _b.reset(new ConstantAnimation(1));
        _a.reset(new ConstantAnimation(1));
        _i.reset();
        _opacityBoundToIntensity = opacityBoundToIntensity;
    }

    void ColorAnimationEffect::SetColorAnimation(AnimationPtr r, AnimationPtr g, AnimationPtr b) {
        _r = r;
        _g = g;
        _b = b;
    }

    void ColorAnimationEffect::SetOpacityAnimation(AnimationPtr a) {
        _a = a;
    }

    void ColorAnimationEffect::SetIntensityAnimation(AnimationPtr i) {
        _i = i;
    }

    void ColorAnimationEffect::SetFixedColor(const Color &color) {
        SetColorAnimation(std::make_shared<ConstantAnimation>(color.GetR()),
            std::make_shared<ConstantAnimation>(color.GetG()),
            std::make_shared<ConstantAnimation>(color.GetB()));
    }

    void ColorAnimationEffect::SetFixedOpacity(double opacity) {
        SetOpacityAnimation(std::make_shared<ConstantAnimation>(opacity));
    }

    AnimationListPtr ColorAnimationEffect::GetAnimations() {
        if (_i != nullptr)
            return AnimationHelper::CreatePtr(_r, _g, _b, _a, _i);

        return AnimationHelper::CreatePtr(_r, _g, _b, _a);
    }

    void ColorAnimationEffect::SetIntensity(Color* color) const {
        if (_i != nullptr)
            color->ApplyBrightness(_i->GetValue());

        if (_opacityBoundToIntensity) {
            auto bri = color->GetCurrentBrightness();
            color->SetAlpha(_a->GetValue() * bri);
        }
    }

    void ColorAnimationEffect::Serialize(JSONNode *node) const {
        AnimationEffect::Serialize(node);

        SerializeAttribute(node, AttributeR, _r);
        SerializeAttribute(node, AttributeG, _g);
        SerializeAttribute(node, AttributeB, _b);
        SerializeAttribute(node, AttributeA, _a);
        SerializeAttribute(node, AttributeI, _i);
        SerializeValue(node, AttributeOpacityBoundToIntensity, _opacityBoundToIntensity);
    }

    void ColorAnimationEffect::Deserialize(JSONNode *node) {
        AnimationEffect::Deserialize(node);

        _r = DeserializeAttribute<Animation>(node, AttributeR, _r);
        _g = DeserializeAttribute<Animation>(node, AttributeG, _g);
        _b = DeserializeAttribute<Animation>(node, AttributeB, _b);
        _a = DeserializeAttribute<Animation>(node, AttributeA, _a);
        _i = DeserializeAttribute<Animation>(node, AttributeI, _i);
        DeserializeValue(node, AttributeOpacityBoundToIntensity, &_opacityBoundToIntensity, false);
    }

}  // namespace huestream
