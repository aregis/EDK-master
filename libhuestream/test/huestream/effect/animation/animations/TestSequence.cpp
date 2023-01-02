#include "gtest/gtest.h"
#include "test/huestream/common/TestSerializeBase.h"
#include <memory>
#include <huestream/effect/animation/data/PointHelper.h>
#include <huestream/effect/animation/animations/base/AnimationHelper.h>
#include <huestream/effect/animation/animations/SequenceAnimation.h>
#include <huestream/effect/animation/animations/CurveAnimation.h>
#include <huestream/effect/animation/animations/TweenAnimation.h>
#include <huestream/effect/animation/animations/RandomAnimation.h>
#include <huestream/effect/animation/animations/ConstantAnimation.h>
#include <huestream/effect/animation/Player.h>
#include <huestream/common/time/TimeManager.h>
#include <huestream/common/time/TimeProviderProvider.h>
#include <huestream/config/ObjectBuilder.h>
#include <huestream/common/data/Bridge.h>

using namespace huestream;

class StubTimeProvider : public TimeManager {
public:
    void AddMilliseconds(int msecs) {
        _now = _now + msecs;
    }
};

class TestSequence : public testing::Test, public TestSerializeBase {
private:
    std::shared_ptr<StubTimeProvider> _tp;
    using ScopedtimeProviderProvider = support::ScopedProvider<TimeProviderPtr>;
    ScopedtimeProviderProvider _tpScope;

public:
    TestSequence()
      : _tp(std::make_shared<StubTimeProvider>()),
        _tpScope(_tp),
        _player(std::make_shared<Player>())
    {}

    std::shared_ptr<Player> _player;
protected:
    virtual void AddMillisecondsAndUpdateMarkers(long milliseconds) {
        _tp->AddMilliseconds(milliseconds);
        _player->UpdateMarkers();
    }

    virtual double AddMillisecondsAndReturnValue(const std::shared_ptr<Animation> &animation, long milliseconds) {
        AddMillisecondsAndUpdateMarkers(milliseconds);
        return animation->GetValue();
    }

    virtual AnimationPtr CreateCurveFromPoints(PointListPtr points, double repeatTimes = 0) {
        return std::make_shared<CurveAnimation>(repeatTimes, points);
    }

    virtual void SetUp() {
        Serializable::SetObjectBuilder(std::make_shared<ObjectBuilder>(std::make_shared<BridgeSettings>()));
    }

    virtual void TearDown() {
        _tp.reset();
        _player.reset();
        Serializable::SetObjectBuilder(std::make_shared<ObjectBuilder>(nullptr));
    }
};

TEST_F(TestSequence, ConcatTwoEndableCurves) {
    auto sequence = std::make_shared<SequenceAnimation>();
    auto curve1 = CreateCurveFromPoints(PointHelper::CreatePtr(NEW_PTR(Point, 0, 1), NEW_PTR(Point, 999, 2)));
    auto curve2 = CreateCurveFromPoints(PointHelper::CreatePtr(NEW_PTR(Point, 0, 3), NEW_PTR(Point, 999, 4)));
    sequence->Append(curve1);
    sequence->Append(curve2);
    _player->BindAnimations(AnimationHelper::CreatePtr(sequence));
    _player->Start();
    EXPECT_EQ(sequence->GetTotalLength(), 2000);
    EXPECT_EQ(AddMillisecondsAndReturnValue(sequence, 0), 1);
    EXPECT_EQ(AddMillisecondsAndReturnValue(sequence, 999), 2);
    EXPECT_EQ(AddMillisecondsAndReturnValue(sequence, 1), 3);
    EXPECT_EQ(AddMillisecondsAndReturnValue(sequence, 999), 4);
    EXPECT_EQ(sequence->IsPlaying(), true);
    EXPECT_EQ(AddMillisecondsAndReturnValue(sequence, 1), 4);
    EXPECT_EQ(sequence->IsPlaying(), false);
    EXPECT_EQ(AddMillisecondsAndReturnValue(sequence, 10000), 4);
}

TEST_F(TestSequence, ConcatTwoEndableCurvesWithRepeat) {
    auto sequence = std::make_shared<SequenceAnimation>();
    auto curve1 = CreateCurveFromPoints(PointHelper::CreatePtr(NEW_PTR(Point, 0, 1), NEW_PTR(Point, 999, 2)), 2);
    auto curve2 = CreateCurveFromPoints(PointHelper::CreatePtr(NEW_PTR(Point, 0, 3), NEW_PTR(Point, 999, 4)));
    sequence->Append(curve1);
    sequence->Append(curve2);
    _player->BindAnimations(AnimationHelper::CreatePtr(sequence));
    _player->Start();
    EXPECT_EQ(sequence->GetTotalLength(), 4000);
    EXPECT_EQ(AddMillisecondsAndReturnValue(sequence, 0), 1);
    EXPECT_EQ(AddMillisecondsAndReturnValue(sequence, 999), 2);
    EXPECT_EQ(AddMillisecondsAndReturnValue(sequence, 1), 1);
    EXPECT_EQ(AddMillisecondsAndReturnValue(sequence, 999), 2);
    EXPECT_EQ(AddMillisecondsAndReturnValue(sequence, 1), 1);
    EXPECT_EQ(AddMillisecondsAndReturnValue(sequence, 999), 2);
    EXPECT_EQ(AddMillisecondsAndReturnValue(sequence, 1), 3);
    EXPECT_EQ(AddMillisecondsAndReturnValue(sequence, 999), 4);
    EXPECT_EQ(sequence->IsPlaying(), true);
    EXPECT_EQ(AddMillisecondsAndReturnValue(sequence, 1), 4);
    EXPECT_EQ(sequence->IsPlaying(), false);
    EXPECT_EQ(AddMillisecondsAndReturnValue(sequence, 10000), 4);
}

TEST_F(TestSequence, ConcatEndlessRepeatWithEndable) {
    auto sequence = std::make_shared<SequenceAnimation>();
    auto curve1 = CreateCurveFromPoints(PointHelper::CreatePtr(NEW_PTR(Point, 0, 1), NEW_PTR(Point, 999, 2)), INF);
    auto curve2 = CreateCurveFromPoints(PointHelper::CreatePtr(NEW_PTR(Point, 0, 3), NEW_PTR(Point, 999, 4)));
    sequence->Append(curve1);
    sequence->Append(curve2);
    _player->BindAnimations(AnimationHelper::CreatePtr(sequence));
    _player->Start();
    EXPECT_EQ(sequence->GetTotalLength(), INF);
    EXPECT_EQ(AddMillisecondsAndReturnValue(sequence, 0), 1);
    EXPECT_EQ(AddMillisecondsAndReturnValue(sequence, 999), 2);
    EXPECT_EQ(AddMillisecondsAndReturnValue(sequence, 1), 1);
    EXPECT_EQ(AddMillisecondsAndReturnValue(sequence, 999), 2);
    EXPECT_EQ(AddMillisecondsAndReturnValue(sequence, 1), 1);
    EXPECT_EQ(AddMillisecondsAndReturnValue(sequence, 999), 2);
}

TEST_F(TestSequence, ConcatEndlessRepeatWithEndableAndJump) {
    auto sequence = std::make_shared<SequenceAnimation>();
    auto curve1 = CreateCurveFromPoints(PointHelper::CreatePtr(NEW_PTR(Point, 0, 1), NEW_PTR(Point, 999, 2)), INF);
    auto curve2 = CreateCurveFromPoints(PointHelper::CreatePtr(NEW_PTR(Point, 0, 3), NEW_PTR(Point, 999, 4)));
    sequence->Append(curve1);
    sequence->Append(curve2, "end");
    _player->BindAnimations(AnimationHelper::CreatePtr(sequence));
    _player->Start();
    EXPECT_EQ(sequence->GetTotalLength(), INF);
    EXPECT_EQ(AddMillisecondsAndReturnValue(sequence, 0), 1);
    EXPECT_EQ(AddMillisecondsAndReturnValue(sequence, 999), 2);
    EXPECT_EQ(AddMillisecondsAndReturnValue(sequence, 1), 1);
    EXPECT_EQ(AddMillisecondsAndReturnValue(sequence, 999), 2);
    EXPECT_EQ(AddMillisecondsAndReturnValue(sequence, 1), 1);
    EXPECT_EQ(AddMillisecondsAndReturnValue(sequence, 999), 2);
    sequence->Trigger("end");
    EXPECT_EQ(sequence->GetTotalLength(), 1000);
    EXPECT_EQ(sequence->GetValue(), 3);
    EXPECT_EQ(AddMillisecondsAndReturnValue(sequence, 0), 3);
    EXPECT_EQ(AddMillisecondsAndReturnValue(sequence, 999), 4);
    EXPECT_EQ(sequence->IsPlaying(), true);
    EXPECT_EQ(AddMillisecondsAndReturnValue(sequence, 1), 4);
    EXPECT_EQ(sequence->IsPlaying(), false);
    EXPECT_EQ(_player->IsStopped(), true);
}

TEST_F(TestSequence, ConcatToSequenceAnimation) {
    auto sequenceleft = std::make_shared<SequenceAnimation>();
    auto sequencetotal = std::make_shared<SequenceAnimation>();
    auto curveleft1 = CreateCurveFromPoints(PointHelper::CreatePtr(NEW_PTR(Point, 0, 1), NEW_PTR(Point, 999, 2)),
                                            INF);
    auto curveleft2 = CreateCurveFromPoints(PointHelper::CreatePtr(NEW_PTR(Point, 0, 3), NEW_PTR(Point, 999, 4)));
    auto curveright2 = CreateCurveFromPoints(
            PointHelper::CreatePtr(NEW_PTR(Point, 0, 100), NEW_PTR(Point, 2999, 200)));
    sequenceleft->Append(curveleft1);
    sequenceleft->Append(curveleft2, "toLeft2");

    sequencetotal->Append(sequenceleft);
    sequencetotal->Append(curveright2);
    _player->BindAnimations(AnimationHelper::CreatePtr(sequencetotal));
    _player->Start();
    EXPECT_EQ(sequenceleft->GetTotalLength(), INF);
    EXPECT_EQ(sequencetotal->GetTotalLength(), INF);
    EXPECT_EQ(AddMillisecondsAndReturnValue(sequencetotal, 0), 1);
    EXPECT_EQ(AddMillisecondsAndReturnValue(sequencetotal, 999), 2);
    EXPECT_EQ(AddMillisecondsAndReturnValue(sequencetotal, 1), 1);
    EXPECT_EQ(AddMillisecondsAndReturnValue(sequencetotal, 999), 2);
    sequenceleft->Trigger("toLeft2");
    EXPECT_EQ(sequenceleft->GetTotalLength(), 1000);
    EXPECT_EQ(sequencetotal->GetTotalLength(), 4000);
    EXPECT_EQ(AddMillisecondsAndReturnValue(sequencetotal, 0), 3);
    EXPECT_EQ(AddMillisecondsAndReturnValue(sequencetotal, 999), 4);
    EXPECT_EQ(AddMillisecondsAndReturnValue(sequencetotal, 1), 100);
    EXPECT_EQ(AddMillisecondsAndReturnValue(sequencetotal, 2999), 200);
    EXPECT_EQ(sequencetotal->IsPlaying(), true);
    EXPECT_EQ(AddMillisecondsAndReturnValue(sequencetotal, 1), 200);
    EXPECT_EQ(sequencetotal->IsPlaying(), false);
    EXPECT_EQ(_player->IsStopped(), true);
}

TEST_F(TestSequence, SerializeDeserialize) {
    auto sequence = std::make_shared<SequenceAnimation>();
    auto curve1 = CreateCurveFromPoints(PointHelper::CreatePtr(NEW_PTR(Point, 0, 1), NEW_PTR(Point, 2, 3)), 1);
    auto curve2 = CreateCurveFromPoints(PointHelper::CreatePtr(NEW_PTR(Point, 4, 5), NEW_PTR(Point, 6, 7)), 2);
    auto tween = std::make_shared<TweenAnimation>(16, 17, 18, TweenType::EaseInOutQuad);
    auto constant = std::make_shared<ConstantAnimation>(12);
    auto nested = std::make_shared<SequenceAnimation>();
    auto curve11 = CreateCurveFromPoints(PointHelper::CreatePtr(NEW_PTR(Point, 0, 1), NEW_PTR(Point, 2, 3)), 1);
    auto curve21 = CreateCurveFromPoints(PointHelper::CreatePtr(NEW_PTR(Point, 4, 5), NEW_PTR(Point, 6, 7)), 2);

    sequence->Append(curve1, "CurveAnimation1");
    sequence->Append(curve2);
    sequence->Append(tween, "TweenAnimation");
    sequence->Append(constant, "Constant");
    sequence->Append(nested);
    nested->Append(curve11, "CurveAnimation1");
    nested->Append(curve21);

    std::string v = sequence->SerializeText();

    std::cout << v << std::endl;
    auto sequence2 = std::make_shared<SequenceAnimation>();
    sequence2->DeserializeText(v);
    ASSERT_EQ(5, sequence2->get_num_sequences());
    ASSERT_EQ(3, sequence2->get_num_triggers());
    ASSERT_EQ(0, sequence2->get_trigger_index("CurveAnimation1").second);
    ASSERT_EQ(2, sequence2->get_trigger_index("TweenAnimation").second);
    ASSERT_EQ(3, sequence2->get_trigger_index("Constant").second);
}

TEST_F(TestSequence, Clone) {
    auto       sequence = std::make_shared<SequenceAnimation>();
    auto sequence_clone = sequence->Clone();
    ASSERT_NE(nullptr, sequence_clone);
    ASSERT_EQ(sequence->GetValue(), sequence_clone->GetValue());
    ASSERT_EQ(sequence->GetTypeName(), sequence_clone->GetTypeName());
    ASSERT_EQ(sequence->GetTotalLength(), sequence_clone->GetTotalLength());
}
