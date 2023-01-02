/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#ifndef HUESTREAM_EFFECT_ANIMATION_PLAYER_H_
#define HUESTREAM_EFFECT_ANIMATION_PLAYER_H_

#include "huestream/effect/animation/animations/base/Animation.h"
#include "huestream/common/time/ITimeProvider.h"
#include "huestream/effect/animation/IPlayer.h"

#include <stdint.h>

#include <vector>
#include <memory>

namespace huestream {

    class Player : public IPlayer {
    public:
        Player();
        explicit Player(AnimationListPtr animations);

        void Start() override;

        void Stop() override;

        void BindAnimations(AnimationListPtr animations) const override;

        void UpdateMarkers() override;

        bool IsStopped() override;

        void SetSpeed(double value) override;

    private:
        AnimationListPtr _animations;
        double _stopValue;
        double _speed;
        int64_t _startPosition;
        int64_t _timeStampWithSpeedCorrection;
        int64_t _oldTs;
        enum class State {
            Running, Stopping, Stopped
        } _state;

        void RewindNewAnimations(AnimationListPtr animations) const;
        double getNowFromStartPosition() const;
        bool AnimationIsBound(AnimationPtr animation) const;
    };

}  // namespace huestream

#endif  // HUESTREAM_EFFECT_ANIMATION_PLAYER_H_
