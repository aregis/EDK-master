/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <huestream/config/Config.h>
#include <huestream/effect/lightscript/Timeline.h>

#include <math.h>

#include <limits>
#include <memory>

namespace huestream {

Timeline::Timeline(const std::shared_ptr<ITimeProvider> &realTimeProvider, int64_t latencyCompensation) :
    _startTime(0),
    _position(0),
    _running(false),
    _realTimeProvider(realTimeProvider),
    _latencyCompensation(latencyCompensation),
    _length(std::numeric_limits<int64_t>::max()) {
}

Timeline::~Timeline() {
    _stateChangedHandler = nullptr;
}

void Timeline::Start() {
    SetPosition(0);
    _running = true;
    OnTimelineStateChanged(TIMELINE_STATE_STARTED);
}

void Timeline::OnTimelineStateChanged(TimelineState s) {
    if (_stateChangedHandler != nullptr) {
        _stateChangedHandler->OnTimelineStateChanged(s, this);
    }

    if (_stateChangedCallback != nullptr) {
        _stateChangedCallback(s, this);
    }
}

void Timeline::Pause() {
    _running = false;
    OnTimelineStateChanged(TIMELINE_STATE_PAUSED);
}

void Timeline::Resume() {
    SetPosition(_position);
    _running = true;
    OnTimelineStateChanged(TIMELINE_STATE_RESUMED);
}

void Timeline::Stop() {
    _running = false;
    SetPosition(0);
    OnTimelineStateChanged(TIMELINE_STATE_STOPPED);
}

void Timeline::End() {
    _running = false;
    OnTimelineStateChanged(TIMELINE_STATE_ENDED);
}


void Timeline::SetPosition(int64_t position) {
    _startTime = _realTimeProvider->Now() - position - _latencyCompensation;
    _position = position;
}

bool Timeline::IsRunning() {
    return _running;
}

int64_t Timeline::Now() {
    if (_running)
        UpdatePosition();

    return _position;
}

int64_t Timeline::GetLength() {
    return _length;
}

void Timeline::SetLength(int64_t length) {
    _length = length;
}

void Timeline::UpdatePosition() {
    auto position = _realTimeProvider->Now() - _startTime;

    if (position > _length) {
        position = _length;
        End();
    }

    _position = position;
}

void Timeline::RegisterTimelineStateChangedHandler(TimelineStateChangedHandlerPtr handler) {
    _stateChangedHandler = handler;
}

void Timeline::RegisterTimelineStateChangedCallback(TimelineStateChangedCallback callback) {
    _stateChangedCallback = callback;
}
}  // namespace huestream
