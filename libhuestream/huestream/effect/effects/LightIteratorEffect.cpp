/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <huestream/common/util/Rand.h>
#include <huestream/effect/animation/data/Point.h>
#include <huestream/effect/animation/data/Vector.h>
#include <huestream/effect/animation/animations/SequenceAnimation.h>
#include <huestream/effect/animation/animations/TweenAnimation.h>
#include <huestream/effect/IMixer.h>
#include <huestream/effect/effects/LightIteratorEffect.h>
#include <huestream/effect/animation/animations/base/AnimationHelper.h>

#include <algorithm>
#include <memory>
#include <string>
#include <map>
#include <utility>

using std::make_shared;

namespace huestream {

const std::map<IterationOrder, std::string> LightIteratorEffect::_orderSerializeMap = {
    {IterationOrderLeftRight, "leftright"},
    {IterationOrderFrontBack, "frontback"},
    {IterationOrderTopBottom, "topbottom"},
    {IterationOrderClockwise, "clockwise"},
    {IterationOrderInOut,     "inout"},
    {IterationOrderRandom,    "random"},
    {IterationOrderGroup,     "group"}
};

const std::map<IterationMode, std::string> LightIteratorEffect::_modeSerializeMap = {
    {IterationModeSingle, "single"},
    {IterationModeCycle,  "cycle"},
    {IterationModeBounce, "bounce"}
};

PROP_IMPL(LightIteratorEffect, IterationOrder, order, Order);
PROP_IMPL(LightIteratorEffect, IterationMode, mode, Mode);
PROP_IMPL(LightIteratorEffect, double, offset, Offset);
PROP_IMPL_BOOL(LightIteratorEffect, bool, preamble, Preamble);
PROP_IMPL_BOOL(LightIteratorEffect, bool, invertOrder, InvertOrder);


LightIteratorEffect::LightIteratorEffect(std::string name,
                                         unsigned int layer,
                                         IterationOrder order,
                                         IterationMode mode,
                                         double offset,
                                         bool preamble,
                                         bool invertOrder) :
    ColorAnimationEffect(name, layer, false),
    _order(order),
    _mode(mode),
    _offset(offset),
    _preamble(preamble),
    _invertOrder(invertOrder),
    _lightIdAnimationMap(std::make_shared<std::map<std::string, AnimationListPtr>>()),
    _group(std::make_shared<Group>()) {
}

LightIteratorEffect::~LightIteratorEffect() {
}

void LightIteratorEffect::UpdateGroup(GroupPtr group) {
    _group = group;
    InitializeAnimations();
}

void LightIteratorEffect::InitializeAnimations() {
    CreateAnimations(_group);
    ColorAnimationEffect::InitializeAnimations();
}

Color LightIteratorEffect::GetColor(LightPtr light) {
    auto it = _lightIdAnimationMap->find(light->GetId());
    if (it != _lightIdAnimationMap->end()) {
        auto r = it->second->at(0)->GetValue();
        auto g = it->second->at(1)->GetValue();
        auto b = it->second->at(2)->GetValue();
        auto a = it->second->at(3)->GetValue();
        auto color = Color(r, g, b, a);
        if (_opacityBoundToIntensity) {
            auto bri = color.GetCurrentBrightness();
            color.SetAlpha(a * bri);
        }
        return color;
    }
    return Color();
}

void LightIteratorEffect::CreateAnimations(GroupPtr group) {
    _lightIdAnimationMap->clear();
    auto lights = make_shared<LightList>(*group->GetLights());

    if (_order != IterationOrderGroup)
        std::sort(lights->begin(), lights->end(), [this](LightPtr l, LightPtr r) { return CompareLights(l, r); });

    auto repeatTimes = _mode == IterationModeSingle ? 0 : INF;

    for (unsigned int i = 0; i < lights->size(); i++) {
        auto r = make_shared<SequenceAnimation>(repeatTimes);
        auto g = make_shared<SequenceAnimation>(repeatTimes);
        auto b = make_shared<SequenceAnimation>(repeatTimes);
        auto a = make_shared<SequenceAnimation>(repeatTimes);

        if (i != 0) {
            auto before = make_shared<TweenAnimation>(0, 0, i * _offset, TweenType::Linear);
            r->Append(before);
            g->Append(before);
            b->Append(before);
            a->Append(before);
        }

        r->Append(_r->Clone());
        g->Append(_g->Clone());
        b->Append(_b->Clone());
        a->Append(_a->Clone());

        if (i != lights->size() - 1) {
            auto after = make_shared<TweenAnimation>(0, 0, (lights->size() - 1 - i) * _offset, TweenType::Linear);
            r->Append(after);
            g->Append(after);
            b->Append(after);
            a->Append(after);
        }

        if (_mode == IterationModeBounce) {
            if (i != lights->size() - 1) {
                auto beforeBounce = make_shared<TweenAnimation>(0, 0, (lights->size() - 1 - i) * _offset,
                                                               TweenType::Linear);
                r->Append(beforeBounce);
                g->Append(beforeBounce);
                b->Append(beforeBounce);
                a->Append(beforeBounce);
            }

            r->Append(_r->Clone());
            g->Append(_g->Clone());
            b->Append(_b->Clone());
            a->Append(_a->Clone());

            if (i != 0) {
                auto afterBounce = make_shared<TweenAnimation>(0, 0, i * _offset, TweenType::Linear);
                r->Append(afterBounce);
                g->Append(afterBounce);
                b->Append(afterBounce);
                a->Append(afterBounce);
            }
        }

        auto animations = AnimationHelper::CreatePtr(r, g, b, a);
        _lightIdAnimationMap->insert(std::pair<std::string, AnimationListPtr>(lights->at(i)->GetId(), animations));
    }
}

void LightIteratorEffect::RenderUpdate() {
}

AnimationListPtr LightIteratorEffect::GetAnimations() {
    auto animations = std::make_shared<AnimationList>();
    for (auto it = _lightIdAnimationMap->begin(); it != _lightIdAnimationMap->end(); ++it) {
        animations->push_back(it->second->at(0));
        animations->push_back(it->second->at(1));
        animations->push_back(it->second->at(2));
        animations->push_back(it->second->at(3));
    }
    return animations;
}

std::string LightIteratorEffect::GetTypeName() const {
    return type;
}

void LightIteratorEffect::Serialize(JSONNode *node) const {
    ColorAnimationEffect::Serialize(node);

    SerializeOrder(node);
    SerializeMode(node);
    SerializeValue(node, AttributeOffset, _offset);
    SerializeValue(node, AttributePreamble, _preamble);
    SerializeValue(node, AttributeInvertOrder, _invertOrder);
}

void LightIteratorEffect::Deserialize(JSONNode *node) {
    ColorAnimationEffect::Deserialize(node);

    DeserializeOrder(node);
    DeserializeMode(node);
    DeserializeValue(node, AttributeOffset, &_offset, 0);
    DeserializeValue(node, AttributePreamble, &_preamble, true);
    DeserializeValue(node, AttributeInvertOrder, &_invertOrder, false);
}

void LightIteratorEffect::SerializeOrder(JSONNode *node) const {
    auto it = _orderSerializeMap.find(_order);
    if (it != _orderSerializeMap.end()) {
        SerializeValue(node, AttributeOrder, it->second);
    } else {
        SerializeValue(node, AttributeOrder, "default");
    }
}

void LightIteratorEffect::SerializeMode(JSONNode *node) const {
    auto it = _modeSerializeMap.find(_mode);
    if (it != _modeSerializeMap.end()) {
        SerializeValue(node, AttributeMode, it->second);
    } else {
        SerializeValue(node, AttributeMode, "default");
    }
}

void LightIteratorEffect::DeserializeOrder(JSONNode *node) {
    std::string orderString;
    DeserializeValue(node, AttributeOrder, &orderString, "default");

    _order = IterationOrderGroup;

    for (auto it = _orderSerializeMap.begin(); it != _orderSerializeMap.end(); ++it) {
        if (it->second == orderString) {
            _order = it->first;
            break;
        }
    }
}

void LightIteratorEffect::DeserializeMode(JSONNode *node) {
    std::string modeString;
    DeserializeValue(node, AttributeMode, &modeString, "default");

    _mode = IterationModeCycle;

    for (auto it = _modeSerializeMap.begin(); it != _modeSerializeMap.end(); ++it) {
        if (it->second == modeString) {
            _mode = it->first;
            break;
        }
    }
}

bool LightIteratorEffect::CompareLights(LightPtr i, LightPtr j) const {
    auto result = false;

    switch (_order) {
        case IterationOrderLeftRight :
            result = i->GetPosition().GetX() < j->GetPosition().GetX();
            break;

        case IterationOrderFrontBack :
            result = j->GetPosition().GetY() < i->GetPosition().GetY();
            break;

        case IterationOrderTopBottom:
            result = j->GetPosition().GetZ() < i->GetPosition().GetZ();
            break;

        case IterationOrderClockwise : {
            auto vectorI = Vector(i->GetPosition().GetX(), i->GetPosition().GetY());
            auto angleLightPointI = vectorI.GetAngle();
            auto vectorJ = Vector(j->GetPosition().GetX(), j->GetPosition().GetY());
            auto angleLightPointJ = vectorJ.GetAngle();
            result = angleLightPointJ < angleLightPointI;
            break;
        }

        case IterationOrderInOut : {
            auto middle = Point(0, 0);
            auto lightPointI = Point(i->GetPosition().GetX(), i->GetPosition().GetY());
            auto vectorI = Vector(middle, lightPointI);
            auto lengthLightPointI = vectorI.GetLength();
            auto lightPointJ = Point(j->GetPosition().GetX(), j->GetPosition().GetY());
            auto vectorJ = Vector(middle, lightPointJ);
            auto lengthLightPointJ = vectorJ.GetLength();
            result = lengthLightPointI < lengthLightPointJ;
            break;
        }

        case IterationOrderRandom :
            result = Rand(0, 1) > 0.5;
            break;

        default:
            result = false;
    }

    if (_invertOrder)
        result = !result;

    return result;
}

}  // namespace huestream
