/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <huestream/effect/effects/ManualEffect.h>

#include <string>
#include <memory>

namespace huestream {

    ManualEffect::ManualEffect(std::string name, unsigned int layer) :
            Effect(name, layer) {
    }

    ManualEffect::~ManualEffect() {
    }

    Color ManualEffect::GetColor(LightPtr light) {
        auto it = _idColorMap.find(light->GetId());
        if (it != _idColorMap.end())
            return it->second;

        return Color();
    }

    void ManualEffect::Render() {
    }

    void ManualEffect::SetIdToColor(std::string id, const Color &color) {
        _idColorMap[id] = color;
    }

    std::string ManualEffect::GetTypeName() const {
        return type;
    }

}  // namespace huestream
