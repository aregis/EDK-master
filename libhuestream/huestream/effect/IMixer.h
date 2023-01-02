/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#ifndef HUESTREAM_EFFECT_IMIXER_H_
#define HUESTREAM_EFFECT_IMIXER_H_

#include "huestream/common/data/Group.h"
#include "huestream/effect/effects/base/Effect.h"

#include <memory>
#include <string>

namespace huestream {

    class IMixer {
    public:
        virtual ~IMixer() = default;

        virtual void AddEffect(EffectPtr effect) = 0;

        virtual void AddEffectList(EffectListPtr effects) = 0;

        virtual GroupPtr GetGroup() = 0;

        virtual void Render() = 0;

        virtual void SetGroup(const GroupPtr group) = 0;

        virtual EffectPtr GetEffectByName(std::string name) = 0;

        virtual void Lock() = 0;

        virtual void Unlock() = 0;
    };

    typedef std::shared_ptr<IMixer> MixerPtr;
}  // namespace huestream

#endif  // HUESTREAM_EFFECT_IMIXER_H_
