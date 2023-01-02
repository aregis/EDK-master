/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <huestream/effect/effects/base/RadialEffect.h>
#include <huestream/config/Config.h>
#include <huestream/common/util/HueMath.h>
#include <huestream/effect/IMixer.h>
#include <huestream/effect/animation/animations/ConstantAnimation.h>
#include <huestream/effect/animation/data/Vector.h>

#include <map>
#include <cmath>
#include <string>
#include <memory>
#include <vector>

namespace huestream {


    PROP_IMPL(RadialEffect, double, angle, Angle);

    RadialEffect::RadialEffect(const std::string &name, unsigned int layer) :
            ColorAnimationEffect(name, layer),
            _angle(0),
            _effect_lights(std::make_shared<LightList>()) {
    }

    void RadialEffect::RenderUpdate() {}

    void RadialEffect::MarkLights(double markPointDegrees) {
        std::map<double, LightPtr> lightMap;
        if (_group == nullptr) {
            return;
        }

        for (auto light : *_group->GetLights()) {
            auto middle = Point(0, 0);
            auto lightPoint = Point(light->GetPosition().GetX(), light->GetPosition().GetY());
            auto vector = Vector(middle, lightPoint);
            auto degreesLightPoint = vector.GetAngle();

            lightMap[degreesLightPoint] = light;
        }

        if (lightMap.size() > 1) {
            auto firstLight = lightMap.begin();
            auto secondLight = --lightMap.end();

            for (auto i = 0; i < static_cast<int>(lightMap.size()) - 1; i++) {
                auto firstLightIt = std::next(lightMap.begin(), i);
                auto secondLightIt = std::next(lightMap.begin(), i + 1);
                auto a = firstLightIt->first;
                auto b = secondLightIt->first;
                if (HueMath::isAngleBetween(markPointDegrees, a, b)) {
                    firstLight = firstLightIt;
                    secondLight = secondLightIt;
                    break;
                }
            }

            auto firstDiff = HueMath::getDiffDegrees(firstLight->first, markPointDegrees);
            auto secondDiff = HueMath::getDiffDegrees(secondLight->first, markPointDegrees);
            auto totalDiff = firstDiff + secondDiff;
            auto quarterDiff = totalDiff / 4;

            if (firstDiff < quarterDiff) {
                _effect_lights->push_back(firstLight->second);
            } else if (secondDiff < quarterDiff) {
                _effect_lights->push_back(secondLight->second);
            } else {
                _effect_lights->push_back(firstLight->second);
                _effect_lights->push_back(secondLight->second);
            }
        } else if (lightMap.size() == 1) {
            auto light = lightMap.begin();
            _effect_lights->push_back(light->second);
        }
    }

    void RadialEffect::Enable() {
        if (_group != nullptr) {
            AnimationEffect::Enable();
            MarkLights(_angle);
        }
    }

    void RadialEffect::UpdateGroup(GroupPtr group) {
        _group = group;
        MarkLights(_angle);
    }

    Color RadialEffect::GetColor(LightPtr light) {
        for (auto effectLight : *_effect_lights) {
            if (light->GetId() == effectLight->GetId()) {
                Color color = Color(_r->GetValue(), _g->GetValue(), _b->GetValue(), _a->GetValue());
                return color;
            }
        }

        return Color();
    }

    std::string RadialEffect::GetTypeName() const {
        return type;
    }

    void RadialEffect::Serialize(JSONNode *node) const {
        ColorAnimationEffect::Serialize(node);
        SerializeValue(node, AttributeAngle, _angle);
    }

    void RadialEffect::Deserialize(JSONNode *node) {
        ColorAnimationEffect::Deserialize(node);
        DeserializeValue(node, AttributeAngle, &_angle, 0);
    }

}  // namespace huestream
