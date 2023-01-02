/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <huestream/effect/Mixer.h>

#include <algorithm>
#include <memory>
#include <iostream>
#include <string>

namespace huestream {

    Mixer::Mixer() : _effects(std::make_shared<EffectList>()),
                _group(std::make_shared<Group>()), _retain_color(false) {
    }

    Mixer::Mixer(AppSettingsPtr appSettings) : _effects(std::make_shared<EffectList>()),
                _group(std::make_shared<Group>()), _retain_color(appSettings->LightsRetainColor()) {
    }

    Mixer::~Mixer() {
        if (_group) {
            _group.reset();
        }

        for (auto effect : *_effects) {
            effect.reset();
        }

        _effects->clear();
    }

    void Mixer::Lock() {
        _mtx.lock();
    }

    void Mixer::Unlock() {
        _mtx.unlock();
    }

    EffectPtr Mixer::GetEffectByName(std::string name) {
        for (auto effect : *_effects) {
            if (effect->GetName() == name) {
                return effect;
            }
        }

        return nullptr;
    }

    int Mixer::FindEffectIndex(const EffectPtr &newEffect) const {
        auto i = 0;

        for (auto effect : *_effects) {
            if (newEffect->GetLayer() < effect->GetLayer()) {
                break;
            }
            i++;
        }

        return i;
    }

    void Mixer::AddEffect(EffectPtr newEffect) {
        if (newEffect == nullptr)
            return;

        newEffect->UpdateGroup(_group);

        auto i = FindEffectIndex(newEffect);
        _effects->insert(_effects->begin() + i, newEffect);
    }

    void Mixer::AddEffectList(EffectListPtr effects) {
        for (auto effect : *effects) {
            AddEffect(effect);
        }
    }

    void Mixer::SetGroup(const GroupPtr group) {
        if (group == nullptr)
            return;

        _group = group;
        for (auto effect : *_effects) {
            effect->UpdateGroup(_group);
        }
    }

    void Mixer::RenderEffects() {
        for (auto effect : *_effects) {
            if (effect->IsEnabled()) {
                effect->Render();
            }
        }
    }

    void Mixer::ApplyEffectsOnLight(LightPtr light) const {
        auto r = 0.0;
        auto g = 0.0;
        auto b = 0.0;

        if (_retain_color) {
            r = light->GetColor().GetR();
            g = light->GetColor().GetG();
            b = light->GetColor().GetB();
        }

        for (auto effect : *_effects) {
            if (effect->IsEnabled()) {
                auto layerColor = effect->GetColor(light);
                auto alpha = layerColor.GetAlpha();
                auto prevLayersAlpha = 1.0 - alpha;

                r *= prevLayersAlpha;
                g *= prevLayersAlpha;
                b *= prevLayersAlpha;

                r += layerColor.GetR() * alpha;
                g += layerColor.GetG() * alpha;
                b += layerColor.GetB() * alpha;
            }
        }

        auto lightColor = Color(r, g, b);
        light->SetColor(lightColor);
    }

    void Mixer::ApplyEffectsOnLights() {
        for (auto light : *_group->GetLights()) {
            ApplyEffectsOnLight(light);
        }
    }

    void Mixer::RemoveFinishedEffects() {
        auto i = _effects->begin();
        while (i != _effects->end()) {
            if (*i == nullptr || (*i)->IsFinished()) {
                i = _effects->erase(i);
            } else {
                ++i;
            }
        }
    }

    void Mixer::Render() {
        RemoveFinishedEffects();
        RenderEffects();
        ApplyEffectsOnLights();
    }

    GroupPtr Mixer::GetGroup() {
        return _group;
    }

}  // namespace huestream
