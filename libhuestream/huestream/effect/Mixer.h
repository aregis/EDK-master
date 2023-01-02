/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#ifndef HUESTREAM_EFFECT_MIXER_H_
#define HUESTREAM_EFFECT_MIXER_H_

#include "huestream/common/data/Group.h"
#include "huestream/effect/IMixer.h"
#include "huestream/config/AppSettings.h"

#include <memory>
#include <mutex>
#include <string>

namespace huestream {

    class Mixer : public IMixer {
    protected:
        EffectListPtr _effects;
        GroupPtr _group;
        bool _retain_color;

        void RenderEffects();

        void ApplyEffectsOnLights();

        void ApplyEffectsOnLight(LightPtr light) const;

        int FindEffectIndex(const EffectPtr &newEffect) const;

        void RemoveFinishedEffects();

        std::recursive_mutex _mtx;

    public:
        Mixer();

        Mixer(AppSettingsPtr appSettings);

        virtual ~Mixer();

        void Render() override;

        void SetGroup(const GroupPtr group) override;

        GroupPtr GetGroup() override;

        void AddEffect(EffectPtr effect) override;

        void AddEffectList(EffectListPtr effects) override;

        EffectPtr GetEffectByName(std::string name) override;

        void Lock() override;

        void Unlock() override;
    };

}  // namespace huestream

#endif  // HUESTREAM_EFFECT_MIXER_H_
