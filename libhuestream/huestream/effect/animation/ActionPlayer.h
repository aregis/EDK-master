/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#ifndef HUESTREAM_EFFECT_ANIMATION_ACTIONPLAYER_H_
#define HUESTREAM_EFFECT_ANIMATION_ACTIONPLAYER_H_

#include "huestream/effect/animation/animations/base/Animation.h"
#include "huestream/effect/animation/IPlayer.h"

#include <stdint.h>

#include <vector>

namespace huestream {

    class ActionPlayer : public IPlayer {
    private:
        AnimationListPtr _animationList;
        double _speed;
        enum class State {
            Running, Stopped
        } _state;

        void RewindNewAnimations(AnimationListPtr animations) const;

        bool AnimationIsBound(AnimationPtr animation) const;

        int64_t _position;

    public:
        ActionPlayer();

        explicit ActionPlayer(AnimationListPtr animations);

        void Start() override;

        void Stop() override;

        void BindAnimations(AnimationListPtr animations) const override;

        void UpdateMarkers() override;

        bool IsStopped() override;

        void SetSpeed(double value) override;

        void SetPosition(int64_t position);
    };

}  // namespace huestream

#endif  // HUESTREAM_EFFECT_ANIMATION_ACTIONPLAYER_H_
