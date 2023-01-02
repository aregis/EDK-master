#include <huestream/common/time/TimeManager.h>
#include <huestream/common/time/TimeProviderProvider.h>
#include <huestream/effect/animation/Player.h>
#include <huestream/effect/animation/animations/CurveAnimation.h>
#include <huestream/effect/animation/animations/base/AnimationHelper.h>
#include <huestream/effect/animation/data/PointHelper.h>
#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "support/util/Provider.h"

using testing::ElementsAre;
using namespace huestream;

class StubTimeProvider : public TimeManager {
public:
    void AddMilliseconds(int msecs) {
        _now = _now + msecs;
    }
};

class TestPlayer : public testing::Test {
public:
    std::shared_ptr<Player> _player;
    std::shared_ptr<StubTimeProvider> _tp;
    using ScopedtimeProviderProvider = support::ScopedProvider<TimeProviderPtr>;
    ScopedtimeProviderProvider _tpScope;

    TestPlayer()
      : _tp(std::make_shared<StubTimeProvider>()),
        _tpScope(_tp),
        _player(std::make_shared<Player>())
    {}

 protected:
   virtual void AddMillisecondsAndUpdateMarkers(int64_t milliseconds) {
        _tp->AddMilliseconds(milliseconds);
        _player->UpdateMarkers();
    }

    virtual double AddMillisecondsAndReturnValue(std::shared_ptr<Animation> animation, int64_t milliseconds) {
        AddMillisecondsAndUpdateMarkers(milliseconds);
        return animation->GetValue();
    }

    virtual std::shared_ptr<Animation> CreateCurveFromPoints(PointListPtr points, bool repeatable = false) {
        double repeatTimes = 0;
        std::shared_ptr<Animation> curve;

        if (repeatable) {
            repeatTimes = INF;
        }

        curve = std::make_shared<CurveAnimation>(repeatTimes, points);

        return curve;
    }

    virtual std::shared_ptr<Animation> CreateCurveFromValuesAndBind(PointListPtr points, bool repeatable = false) {
        auto curve = CreateCurveFromPoints(points, repeatable);
        _player->BindAnimations(AnimationHelper::CreatePtr(curve));
        return curve;
    }

    virtual void TearDown() {
        _tp.reset();
        _player.reset();
    }
};

TEST_F(TestPlayer, GetFirstSampleWhenNotPlayed) {
    auto curve = CreateCurveFromValuesAndBind(
            PointHelper::CreatePtr(NEW_PTR(Point, 0, 10), NEW_PTR(Point, 1000, 11), NEW_PTR(Point, 2000, 12),
                                   NEW_PTR(Point, 3000, 13), NEW_PTR(Point, 4000, 14), NEW_PTR(Point, 5000, 15)));
    EXPECT_EQ(AddMillisecondsAndReturnValue(curve, 0), 10);
    EXPECT_EQ(AddMillisecondsAndReturnValue(curve, 3000), 10);
    EXPECT_EQ(AddMillisecondsAndReturnValue(curve, 5000), 10);
    EXPECT_EQ(AddMillisecondsAndReturnValue(curve, 7000), 10);
}

TEST_F(TestPlayer, GetLastSampleAfterPlayed) {
    auto curve = CreateCurveFromValuesAndBind(
            PointHelper::CreatePtr(NEW_PTR(Point, 0, 10), NEW_PTR(Point, 1000, 11), NEW_PTR(Point, 2000, 12),
                                   NEW_PTR(Point, 3000, 13), NEW_PTR(Point, 4000, 14), NEW_PTR(Point, 5000, 15)));
    _player->Start();
    EXPECT_EQ(AddMillisecondsAndReturnValue(curve, 5000), 15);
    EXPECT_EQ(AddMillisecondsAndReturnValue(curve, 6000), 15);
}

TEST_F(TestPlayer, GetSamplesInBetweenAfterStartPlaying) {
    auto curve = CreateCurveFromValuesAndBind(
            PointHelper::CreatePtr(NEW_PTR(Point, 0, 10), NEW_PTR(Point, 1000, 11), NEW_PTR(Point, 2000, 12),
                                   NEW_PTR(Point, 3000, 13), NEW_PTR(Point, 4000, 14), NEW_PTR(Point, 5000, 15)));
    _player->Start();
    EXPECT_EQ(AddMillisecondsAndReturnValue(curve, 0), 10);
    EXPECT_EQ(AddMillisecondsAndReturnValue(curve, 250), 10.25);
    EXPECT_EQ(AddMillisecondsAndReturnValue(curve, 250), 10.5);
    EXPECT_EQ(AddMillisecondsAndReturnValue(curve, 500), 11);
    EXPECT_EQ(AddMillisecondsAndReturnValue(curve, 500), 11.5);
    EXPECT_EQ(AddMillisecondsAndReturnValue(curve, 500), 12);
    EXPECT_EQ(AddMillisecondsAndReturnValue(curve, 1000), 13);
    EXPECT_EQ(AddMillisecondsAndReturnValue(curve, 1000), 14);
    EXPECT_EQ(AddMillisecondsAndReturnValue(curve, 1000), 15);
    EXPECT_EQ(AddMillisecondsAndReturnValue(curve, 1000), 15);
}

TEST_F(TestPlayer, ValuesAreSameAfterStopPlaying) {
    auto curve = CreateCurveFromValuesAndBind(
            PointHelper::CreatePtr(NEW_PTR(Point, 0, 10), NEW_PTR(Point, 1000, 11), NEW_PTR(Point, 2000, 12),
                                   NEW_PTR(Point, 3000, 13), NEW_PTR(Point, 4000, 14), NEW_PTR(Point, 5000, 15)));
    _player->Start();
    EXPECT_EQ(AddMillisecondsAndReturnValue(curve, 0), 10);
    EXPECT_EQ(AddMillisecondsAndReturnValue(curve, 1000), 11);
    EXPECT_EQ(AddMillisecondsAndReturnValue(curve, 1000), 12);
    _player->Stop();
    EXPECT_EQ(AddMillisecondsAndReturnValue(curve, 5000), 12);
}

TEST_F(TestPlayer, FirstValueAfterRestarting) {
    auto curve = CreateCurveFromValuesAndBind(
            PointHelper::CreatePtr(NEW_PTR(Point, 0, 10), NEW_PTR(Point, 1000, 11), NEW_PTR(Point, 2000, 12),
                                   NEW_PTR(Point, 3000, 13), NEW_PTR(Point, 4000, 14), NEW_PTR(Point, 5000, 15)));
    _player->Start();
    EXPECT_EQ(AddMillisecondsAndReturnValue(curve, 0), 10);
    EXPECT_EQ(AddMillisecondsAndReturnValue(curve, 1000), 11);
    EXPECT_EQ(AddMillisecondsAndReturnValue(curve, 1000), 12);
    _player->Start();
    EXPECT_EQ(AddMillisecondsAndReturnValue(curve, 0), 10);
    EXPECT_EQ(AddMillisecondsAndReturnValue(curve, 1000), 11);
    EXPECT_EQ(AddMillisecondsAndReturnValue(curve, 1000), 12);
    EXPECT_EQ(AddMillisecondsAndReturnValue(curve, 1000), 13);
    EXPECT_EQ(AddMillisecondsAndReturnValue(curve, 1000), 14);
    EXPECT_EQ(AddMillisecondsAndReturnValue(curve, 1000), 15);
    EXPECT_EQ(AddMillisecondsAndReturnValue(curve, 1), 15);
    EXPECT_EQ(AddMillisecondsAndReturnValue(curve, 1000), 15);
}

TEST_F(TestPlayer, LoopValues) {
    auto curve = CreateCurveFromValuesAndBind(
            PointHelper::CreatePtr(NEW_PTR(Point, 0, 10), NEW_PTR(Point, 1000, 11), NEW_PTR(Point, 1999, 11.999)),
            true);
    _player->Start();
    EXPECT_EQ(AddMillisecondsAndReturnValue(curve, 0), 10);
    EXPECT_EQ(AddMillisecondsAndReturnValue(curve, 1000), 11);
    EXPECT_EQ(AddMillisecondsAndReturnValue(curve, 999), 11.999);

    EXPECT_EQ(AddMillisecondsAndReturnValue(curve, 1), 10);
    EXPECT_EQ(AddMillisecondsAndReturnValue(curve, 1000), 11);
    EXPECT_EQ(AddMillisecondsAndReturnValue(curve, 999), 11.999);

    EXPECT_EQ(AddMillisecondsAndReturnValue(curve, 1), 10);
    EXPECT_EQ(AddMillisecondsAndReturnValue(curve, 1000), 11);
    EXPECT_EQ(AddMillisecondsAndReturnValue(curve, 999), 11.999);

    EXPECT_EQ(AddMillisecondsAndReturnValue(curve, 1), 10);
}

TEST_F(TestPlayer, CurvePlayesSlowerAndFasterWhenSpeedIsChanged) {
    auto curve = CreateCurveFromValuesAndBind(
            PointHelper::CreatePtr(NEW_PTR(Point, 0, 10), NEW_PTR(Point, 1000, 11), NEW_PTR(Point, 2000, 12),
                                   NEW_PTR(Point, 3000, 13), NEW_PTR(Point, 4000, 14), NEW_PTR(Point, 5000, 15)));
    _player->Start();
    EXPECT_EQ(AddMillisecondsAndReturnValue(curve, 0), 10);
    EXPECT_EQ(AddMillisecondsAndReturnValue(curve, 250), 10.25);
    EXPECT_EQ(AddMillisecondsAndReturnValue(curve, 250), 10.5);
    EXPECT_EQ(AddMillisecondsAndReturnValue(curve, 500), 11);
    _player->SetSpeed(0.5);
    EXPECT_EQ(AddMillisecondsAndReturnValue(curve, 1000), 11.5);
    _player->SetSpeed(0);
    EXPECT_EQ(AddMillisecondsAndReturnValue(curve, 1000), 11.5);
    EXPECT_EQ(AddMillisecondsAndReturnValue(curve, 2000), 11.5);
    EXPECT_EQ(AddMillisecondsAndReturnValue(curve, 8000), 11.5);
    _player->SetSpeed(0.5);
    EXPECT_EQ(AddMillisecondsAndReturnValue(curve, 1000), 12);
    EXPECT_EQ(AddMillisecondsAndReturnValue(curve, 2000), 13);
    _player->SetSpeed(2);
    EXPECT_EQ(AddMillisecondsAndReturnValue(curve, 500), 14);
    EXPECT_EQ(AddMillisecondsAndReturnValue(curve, 500), 15);
    EXPECT_EQ(AddMillisecondsAndReturnValue(curve, 500), 15);
}


TEST_F(TestPlayer, PlayerIsStoppedAfterCurvesArePlayed) {
    auto curve = CreateCurveFromPoints(
            PointHelper::CreatePtr(NEW_PTR(Point, 0, 10), NEW_PTR(Point, 1000, 11)));
    auto curve2 = CreateCurveFromPoints(
            PointHelper::CreatePtr(NEW_PTR(Point, 0, 10), NEW_PTR(Point, 2000, 11)));
    _player->BindAnimations(AnimationHelper::CreatePtr(curve, curve2));

    EXPECT_EQ(_player->IsStopped(), true);
    _player->Start();
    EXPECT_EQ(_player->IsStopped(), false);
    AddMillisecondsAndUpdateMarkers(1000);
    EXPECT_EQ(_player->IsStopped(), false);
    AddMillisecondsAndUpdateMarkers(1000);
    EXPECT_EQ(_player->IsStopped(), false);
    AddMillisecondsAndUpdateMarkers(1);
    EXPECT_EQ(_player->IsStopped(), true);
}

TEST_F(TestPlayer, PlayerIsNeverStoppedIfOnlyIncludesInfiniteCurveAnimation) {
    auto curve = CreateCurveFromPoints(
            PointHelper::CreatePtr(NEW_PTR(Point, 0, 10), NEW_PTR(Point, 1000, 11)), true);
    auto curve2 = CreateCurveFromPoints(
            PointHelper::CreatePtr(NEW_PTR(Point, 0, 10), NEW_PTR(Point, 2000, 11)), true);
    _player->BindAnimations(AnimationHelper::CreatePtr(curve, curve2));

    EXPECT_EQ(_player->IsStopped(), true);
    _player->Start();
    EXPECT_EQ(_player->IsStopped(), false);
    AddMillisecondsAndUpdateMarkers(1000);
    EXPECT_EQ(_player->IsStopped(), false);
    AddMillisecondsAndUpdateMarkers(1000);
    EXPECT_EQ(_player->IsStopped(), false);
    AddMillisecondsAndUpdateMarkers(1);
    EXPECT_EQ(_player->IsStopped(), false);
    AddMillisecondsAndUpdateMarkers(1000000000);
    EXPECT_EQ(_player->IsStopped(), false);
}

TEST_F(TestPlayer, PlayerIsStoppedAfterMixedCurveTypesAreArePlayedOnFiniteCurvePosition) {
    auto curve = CreateCurveFromPoints(
            PointHelper::CreatePtr(NEW_PTR(Point, 0, 10), NEW_PTR(Point, 1000, 11)));
    auto curve2 = CreateCurveFromPoints(
            PointHelper::CreatePtr(NEW_PTR(Point, 0, 10), NEW_PTR(Point, 2000, 11)), true);
    _player->BindAnimations(AnimationHelper::CreatePtr(curve, curve2));

    EXPECT_EQ(_player->IsStopped(), true);
    _player->Start();
    EXPECT_EQ(_player->IsStopped(), false);
    AddMillisecondsAndUpdateMarkers(1000);
    EXPECT_EQ(_player->IsStopped(), false);
    AddMillisecondsAndUpdateMarkers(1);
    EXPECT_EQ(_player->IsStopped(), true);
}

TEST_F(TestPlayer, CreatePlayer) {
    _player.reset();
    auto curve = CreateCurveFromPoints(PointHelper::CreatePtr(NEW_PTR(Point, 0, 10), NEW_PTR(Point, 1000, 11), NEW_PTR(Point, 2000, 12),
                                                              NEW_PTR(Point, 3000, 13), NEW_PTR(Point, 4000, 14), NEW_PTR(Point, 5000, 15)), false);


    _player =  std::make_shared<Player>(AnimationHelper::CreatePtr(curve));
    EXPECT_EQ(_player->IsStopped(), true);
}

