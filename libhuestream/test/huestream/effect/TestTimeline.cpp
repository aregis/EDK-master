#include <huestream/common/time/TimeManager.h>
#include <huestream/common/time/TimeProviderProvider.h>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "huestream/effect/lightscript/Timeline.h"

using testing::ElementsAre;
using namespace huestream;

class StubTimeProvider : public TimeManager {
public:
    void AddMilliseconds(int msecs) {
        _now = _now + msecs;
    }
};

class MyHandler : public ITimelineStateChangedHandler {
 public:
    std::weak_ptr<ITimeline> _timeline;
    TimelineState _onTimelineStateChangedState;
    ITimeline* _onTimelineStateChangedTimeline;
    int _eventCount;

    MyHandler(std::weak_ptr<ITimeline> timeline) {
        _timeline = std::move(timeline);
        _eventCount = 0;
        _onTimelineStateChangedTimeline = nullptr;
    }

    void AssertNoTimelineStateChanged() {
        ASSERT_EQ(0, _eventCount);
    }

    void AssertTimelineStateChanged(TimelineState state) {
        ASSERT_EQ(1, _eventCount);
        auto timeline_handle = _timeline.lock();
        ASSERT_EQ(_onTimelineStateChangedTimeline, timeline_handle.get());
        ASSERT_EQ(_onTimelineStateChangedState, state);
        _eventCount = 0;
    }

    void OnTimelineStateChanged(TimelineState state, ITimeline* timeline) override {
        _onTimelineStateChangedTimeline = timeline;
        _onTimelineStateChangedState = state;
        _eventCount++;
    }
};

class TestTimeline : public testing::Test{
 public:
    std::shared_ptr<StubTimeProvider> _tp;
    std::shared_ptr<Timeline> _timeline;

    virtual void AddMilliseconds(long milliseconds) {
        _tp->AddMilliseconds(milliseconds);
    }

    virtual void SetUp() {
        _tp = std::make_shared<StubTimeProvider>();
        _timeline = std::make_shared<Timeline>(_tp);
        _myHandler = std::make_shared<MyHandler>(_timeline);
    }

    virtual void TearDown() {
        _tp.reset();
        _timeline.reset();
        _myHandler.reset();
    }

    std::shared_ptr<MyHandler> _myHandler;

    void NofiticationScenario1() {
        _myHandler->AssertNoTimelineStateChanged();
        _timeline->SetLength(1000);
        _timeline->Start();
        _myHandler->AssertTimelineStateChanged(TIMELINE_STATE_STARTED);

        _timeline->Now();
        _myHandler->AssertNoTimelineStateChanged();
        AddMilliseconds(200);
        _myHandler->AssertNoTimelineStateChanged();
        _timeline->Now();
        _myHandler->AssertNoTimelineStateChanged();

        _timeline->Pause();
        _myHandler->AssertTimelineStateChanged(TIMELINE_STATE_PAUSED);

        AddMilliseconds(150);
        _myHandler->AssertNoTimelineStateChanged();
        _timeline->Now();
        _myHandler->AssertNoTimelineStateChanged();

        _timeline->Resume();
        _myHandler->AssertTimelineStateChanged(TIMELINE_STATE_RESUMED);

        AddMilliseconds(100);
        _myHandler->AssertNoTimelineStateChanged();
        _timeline->Now();
        _myHandler->AssertNoTimelineStateChanged();

        _timeline->SetPosition(310);
        _myHandler->AssertNoTimelineStateChanged();
        _timeline->Now();
        _myHandler->AssertNoTimelineStateChanged();

        AddMilliseconds(160);
        _myHandler->AssertNoTimelineStateChanged();
        _timeline->Now();
        _myHandler->AssertNoTimelineStateChanged();

        _timeline->SetPosition(120);
        _myHandler->AssertNoTimelineStateChanged();
        _timeline->Now();
        _myHandler->AssertNoTimelineStateChanged();

        AddMilliseconds(70);
        _myHandler->AssertNoTimelineStateChanged();
        _timeline->Now();
        _myHandler->AssertNoTimelineStateChanged();

        _timeline->Stop();
        _myHandler->AssertTimelineStateChanged(TIMELINE_STATE_STOPPED);
        _myHandler->AssertNoTimelineStateChanged();
        AddMilliseconds(600);
        _myHandler->AssertNoTimelineStateChanged();
        _timeline->Now();
        _myHandler->AssertNoTimelineStateChanged();

        _timeline->SetPosition(950);
        _myHandler->AssertNoTimelineStateChanged();
        _timeline->Now();
        _myHandler->AssertNoTimelineStateChanged();

        _timeline->Resume();
        _myHandler->AssertTimelineStateChanged(TIMELINE_STATE_RESUMED);
        AddMilliseconds(100);
        _timeline->Now();
        _myHandler->AssertTimelineStateChanged(TIMELINE_STATE_ENDED);
        _timeline->Now();
        _myHandler->AssertNoTimelineStateChanged();
    }
};

TEST_F(TestTimeline, ControlTime) {
    _timeline->SetLength(1000);
    ASSERT_FALSE(_timeline->IsRunning());

    _timeline->Start();
    ASSERT_TRUE(_timeline->IsRunning());
    ASSERT_EQ(_timeline->Now(), 0);
    AddMilliseconds(200);
    ASSERT_EQ(_timeline->Now(), 200);

    _timeline->Pause();
    ASSERT_FALSE(_timeline->IsRunning());
    ASSERT_EQ(_timeline->Now(), 200);
    AddMilliseconds(150);
    ASSERT_EQ(_timeline->Now(), 200);

    _timeline->Resume();
    ASSERT_TRUE(_timeline->IsRunning());
    ASSERT_EQ(_timeline->Now(), 200);
    AddMilliseconds(100);
    ASSERT_EQ(_timeline->Now(), 300);

    _timeline->SetPosition(310);
    ASSERT_TRUE(_timeline->IsRunning());
    ASSERT_EQ(_timeline->Now(), 310);
    AddMilliseconds(160);
    ASSERT_EQ(_timeline->Now(), 470);

    _timeline->SetPosition(120);
    ASSERT_TRUE(_timeline->IsRunning());
    ASSERT_EQ(_timeline->Now(), 120);
    AddMilliseconds(70);
    ASSERT_EQ(_timeline->Now(), 190);

    _timeline->Stop();
    ASSERT_FALSE(_timeline->IsRunning());
    ASSERT_EQ(_timeline->Now(), 0);
    AddMilliseconds(600);
    ASSERT_EQ(_timeline->Now(), 0);

    _timeline->SetPosition(950);
    ASSERT_FALSE(_timeline->IsRunning());
    ASSERT_EQ(_timeline->Now(), 950);

    _timeline->Resume();
    ASSERT_TRUE(_timeline->IsRunning());
    AddMilliseconds(100);
    ASSERT_EQ(_timeline->Now(), 1000);
    ASSERT_FALSE(_timeline->IsRunning());
}

TEST_F(TestTimeline, TimelineStateChangedHandler) {
    _timeline->RegisterTimelineStateChangedHandler(_myHandler);
    NofiticationScenario1();
}

TEST_F(TestTimeline, TimelineStateChangedCallback) {
    _timeline->RegisterTimelineStateChangedCallback([this](TimelineState state, ITimeline* timeline){
        _myHandler->OnTimelineStateChanged(state, timeline);
    });
    NofiticationScenario1();
}

