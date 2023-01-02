/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#ifndef HUESTREAM_EFFECT_LIGHTSCRIPT_ITIMELINE_H_
#define HUESTREAM_EFFECT_LIGHTSCRIPT_ITIMELINE_H_

#include "huestream/common/time/ITimeProvider.h"
#include <functional>
#include <memory>

namespace huestream {

class ITimeline;
class ITimelineStateChangedHandler;
typedef std::shared_ptr<ITimelineStateChangedHandler> TimelineStateChangedHandlerPtr;

/**
 * Timeline states that can be sent through ITimelineStateChangedHandler or TimelineStateCallback
 */
enum TimelineState {
    TIMELINE_STATE_STARTED,
    TIMELINE_STATE_PAUSED,
    TIMELINE_STATE_RESUMED,
    TIMELINE_STATE_STOPPED,
    TIMELINE_STATE_ENDED,
};

/**
 * Timeline state changed callback that get called when Timeline state changes
 */
typedef std::function<void(TimelineState state, ITimeline* timeline)> TimelineStateChangedCallback;

/**
 * Timeline state changed handler interface that get called when Timeline state changes
 */
class ITimelineStateChangedHandler {
 public:
    ITimelineStateChangedHandler() = default;
    virtual ~ITimelineStateChangedHandler() = default;

    virtual void OnTimelineStateChanged(TimelineState state, ITimeline* timeline) = 0;
};


/**
 a user controllable progression of time
 @note example usage is binding a huestream::LightScript to a timeline
 */
class ITimeline : public ITimeProvider {
 public:
    /**
     start time progression from beginning
     */
    virtual void Start() = 0;

    /**
     pause time progression but remember current time
     */
    virtual void Pause() = 0;

    /**
     resume time progression from current time
     */
    virtual void Resume() = 0;

    /**
     stop time progression and reset time to beginning
     */
    virtual void Stop() = 0;

    /**
     set position of current time
     @param position Position in milliseconds
     */
    virtual void SetPosition(int64_t position) = 0;

    /**
     check if timeline running, i.e. time is progressing automatically
     */
    virtual bool IsRunning() = 0;

    /**
     set the end position of the timeline, at which point it automatically stops running
     @param length Length in milliseconds
     */
    virtual void SetLength(int64_t length) = 0;

    /**
     get the end position of the timeline
     @return Length in milliseconds
     */
    virtual int64_t GetLength() = 0;


    /**
     set object which handles Timeline state changes.
     */
    virtual void RegisterTimelineStateChangedHandler(TimelineStateChangedHandlerPtr handler) = 0;

    /**
     set callback which handles Timeline state changes.
     @param handler Reference to object which handles feedback from the library
     */
    virtual void RegisterTimelineStateChangedCallback(TimelineStateChangedCallback callback) = 0;
};

/**
 shared pointer to an huestream::ITimeline object
 */
typedef std::shared_ptr<ITimeline> TimelinePtr;

}  // namespace huestream

#endif  // HUESTREAM_EFFECT_LIGHTSCRIPT_ITIMELINE_H_
