/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <huestream/config/Config.h>
#include <huestream/effect/animation/animations/base/Animation.h>
#include <huestream/effect/animation/ActionPlayer.h>

#include <math.h>

#include <memory>

using std::make_shared;

namespace huestream {

    ActionPlayer::ActionPlayer() :
            _animationList(make_shared<AnimationList>()),
            _speed(1),
            _state(State::Stopped),
            _position(0) {}

    ActionPlayer::ActionPlayer(AnimationListPtr animations) :
            _animationList(animations),
            _speed(1),
            _state(State::Stopped),
            _position(0) {}

    bool ActionPlayer::AnimationIsBound(AnimationPtr animation) const {
        for (auto &currAnimation : *_animationList) {
            if (currAnimation == animation) {
                return true;
            }
        }
        return false;
    }

    void ActionPlayer::RewindNewAnimations(AnimationListPtr animations) const {
        for (auto &animation : *animations) {
            if (!AnimationIsBound(animation)) {
                animation->Rewind();
            }
        }
    }

    void ActionPlayer::BindAnimations(AnimationListPtr animations) const {
        RewindNewAnimations(animations);
        *_animationList = *animations;
    }

    void ActionPlayer::Start() {
        _state = State::Running;
    }

    void ActionPlayer::Stop() {
        _state = State::Stopped;
    }

    bool ActionPlayer::IsStopped() {
        UpdateMarkers();
        return (_state == State::Stopped);
    }

    void ActionPlayer::UpdateMarkers() {
        if (_state != State::Stopped) {
            for (auto &animation : *_animationList) {
                auto markerPosition = _position * _speed;

                animation->SetMarker(markerPosition);
            }
        }
    }

    void ActionPlayer::SetSpeed(double value) {
        _speed = value;
    }

    void ActionPlayer::SetPosition(int64_t position) {
        _position = position;
    }
}  // namespace huestream
