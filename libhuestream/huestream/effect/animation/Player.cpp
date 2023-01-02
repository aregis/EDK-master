/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <huestream/config/Config.h>
#include <huestream/effect/animation/animations/base/Animation.h>
#include <huestream/common/time/TimeManager.h>
#include <huestream/common/time/TimeProviderProvider.h>
#include <huestream/effect/animation/Player.h>

#include <math.h>

#include <memory>

using std::make_shared;

namespace huestream {

    Player::Player() :
            _animations(make_shared<AnimationList>()),
            _stopValue(0),
            _speed(1),
            _startPosition(0),
            _timeStampWithSpeedCorrection(0),
            _oldTs(0),
            _state(State::Stopped) {
    }

    Player::Player(AnimationListPtr animations) :
            _animations(animations),
            _stopValue(0),
            _speed(1),
            _startPosition(0),
            _timeStampWithSpeedCorrection(0),
            _oldTs(0),
            _state(State::Stopped) {}

    bool Player::AnimationIsBound(AnimationPtr animation) const {
        for (auto &currAnimation : *_animations) {
            if (currAnimation == animation) {
                return true;
            }
        }
        return false;
    }

    void Player::RewindNewAnimations(AnimationListPtr animations) const {
        for (auto &animation : *animations) {
            if (!AnimationIsBound(animation)) {
                animation->Rewind();
            }
        }
    }

    void Player::BindAnimations(AnimationListPtr animations) const {
        RewindNewAnimations(animations);
        *_animations = *animations;
    }

    void Player::Start() {
        auto timeProvider = TimeProviderProvider::get();
        auto diffTs = timeProvider->Now() - _oldTs;
        _timeStampWithSpeedCorrection += llround(diffTs * _speed);
        _oldTs = timeProvider->Now();

        _startPosition = _timeStampWithSpeedCorrection;
        for (auto &animation : *_animations) {
            animation->Rewind();
        }
        _state = State::Running;
    }

    void Player::Stop() {
        _stopValue = getNowFromStartPosition();
        _state = State::Stopping;
    }

    bool Player::IsStopped() {
        UpdateMarkers();
        return (_state == State::Stopped);
    }

    void Player::UpdateMarkers() {
        auto timeProvider = TimeProviderProvider::get();
        auto diffTs = timeProvider->Now() - _oldTs;
        _timeStampWithSpeedCorrection += llround(diffTs * _speed);
        _oldTs = timeProvider->Now();

        auto allEndingAnimationsAreStopped = true;
        auto endingAnimationPresent = false;

        if (_state != State::Stopped) {
            for (auto &animation : *_animations) {
                auto markerPosition = getNowFromStartPosition();
                auto animationIsEndless = animation->IsEndless();

                if (_state == State::Stopping) {
                    markerPosition = _stopValue;
                }

                animation->SetMarker(markerPosition);

                if (!animationIsEndless) {
                    endingAnimationPresent = true;
                    if (animation->IsPlaying() && allEndingAnimationsAreStopped) {
                        allEndingAnimationsAreStopped = false;
                    }
                }
            }

            if ((allEndingAnimationsAreStopped && endingAnimationPresent) || _state == State::Stopping) {
                _state = State::Stopped;
            }
        }
    }

    double Player::getNowFromStartPosition() const {
        return static_cast<double>(_timeStampWithSpeedCorrection - _startPosition);
    }

    void Player::SetSpeed(double value) {
        _speed = value;
    }
}  // namespace huestream
