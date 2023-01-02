/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#ifndef HUESTREAM_EFFECT_LIGHTSCRIPT_TIMELINE_H_
#define HUESTREAM_EFFECT_LIGHTSCRIPT_TIMELINE_H_

#include "huestream/effect/lightscript/ITimeline.h"

#include <stdint.h>

#include <memory>

namespace huestream {


class Timeline : public ITimeline {
 public:
    explicit Timeline(const std::shared_ptr<ITimeProvider> &realTimeProvider, int64_t latencyCompensation = 0);
    ~Timeline() override;

    void Start() override;

    void Pause() override;

    void Resume() override;

    void Stop() override;

    void SetPosition(int64_t position) override;

    bool IsRunning() override;

    int64_t Now() override;

    void SetLength(int64_t length) override;

    int64_t GetLength() override;

    /**
     set object which handles state update events from the timeline
     @param handler reference to object which handles events
    */
    void RegisterTimelineStateChangedHandler(TimelineStateChangedHandlerPtr handler) override;

    /**
    set callback which handles state update events from the timeline
    @param callback which handles events
    */
    void RegisterTimelineStateChangedCallback(TimelineStateChangedCallback callback) override;

 private:
    int64_t _startTime;
    int64_t _position;
    bool _running;
    std::shared_ptr<ITimeProvider> _realTimeProvider;
    int64_t _latencyCompensation;
    int64_t _length;

    void UpdatePosition();
    TimelineStateChangedHandlerPtr _stateChangedHandler;
    void OnTimelineStateChanged(TimelineState s);
    void End();
    TimelineStateChangedCallback _stateChangedCallback;
};

}  // namespace huestream

#endif  // HUESTREAM_EFFECT_LIGHTSCRIPT_TIMELINE_H_
