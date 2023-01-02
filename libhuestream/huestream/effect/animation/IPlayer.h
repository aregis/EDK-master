/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#ifndef HUESTREAM_EFFECT_ANIMATION_IPLAYER_H_
#define HUESTREAM_EFFECT_ANIMATION_IPLAYER_H_

#include "huestream/effect/animation/animations/base/Animation.h"
#include "huestream/common/time/ITimeProvider.h"

#include <stdint.h>

#include <vector>
#include <memory>

namespace huestream {

    class IPlayer {
    public:
        virtual ~IPlayer() = default;

        virtual void Start() = 0;

        virtual void Stop() = 0;

        virtual void BindAnimations(AnimationListPtr animations) const = 0;

        virtual void UpdateMarkers() = 0;

        virtual bool IsStopped() = 0;

        virtual void SetSpeed(double value) = 0;
     };

    typedef std::shared_ptr<IPlayer> PlayerPtr;

}  // namespace huestream

#endif  // HUESTREAM_EFFECT_ANIMATION_IPLAYER_H_
