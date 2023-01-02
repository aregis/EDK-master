#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <huestream/common/time/TimeManager.h>
#include <huestream/effect/animation/animations/base/AnimationHelper.h>
#include <huestream/effect/animation/data/PointHelper.h>
#include <huestream/effect/animation/ActionPlayer.h>
#include <huestream/effect/animation/animations/CurveAnimation.h>

#include <memory>


namespace huestream {

class TestActionPlayer : public testing::Test {
 public:
    std::shared_ptr<ActionPlayer> _actionPlayer;

 protected:
    virtual void SetUp() {
        _actionPlayer = std::make_shared<ActionPlayer>();
    }

    virtual void TearDown() {
        _actionPlayer.reset();
    }
};

TEST_F(TestActionPlayer, GetMarker) {
    int64_t position = 1;
    double repeatTimes = 0;
    double speed_1 = 0;
    double speed_2 = 2;
    auto animation = std::make_shared<CurveAnimation>(repeatTimes,
                                             PointHelper::CreatePtr(
                                                                    NEW_PTR(Point, 0, 10),
                                                                    NEW_PTR(Point, 1000, 11)));

    double expect_1 = repeatTimes * speed_1;
    _actionPlayer->BindAnimations(AnimationHelper::CreatePtr(animation));

    EXPECT_EQ(_actionPlayer->IsStopped(), true);
    EXPECT_EQ(animation->GetMarker(), expect_1);
    _actionPlayer->Start();
    EXPECT_EQ(animation->GetMarker(), expect_1);
    _actionPlayer->IsStopped();
    EXPECT_EQ(animation->GetMarker(), expect_1);
    _actionPlayer->Stop();

    EXPECT_EQ(_actionPlayer->IsStopped(), true);

    double expect_2 = position * speed_2;
    _actionPlayer->SetSpeed(speed_2);
    _actionPlayer->SetPosition(position);
    _actionPlayer->Start();
    EXPECT_EQ(animation->GetMarker(), expect_1);
    EXPECT_EQ(_actionPlayer->IsStopped(), false);
    EXPECT_EQ(animation->GetMarker(), expect_2);
    _actionPlayer->Stop();
    EXPECT_EQ(animation->GetMarker(), expect_2);
    EXPECT_EQ(_actionPlayer->IsStopped(), true);

}

TEST_F(TestActionPlayer, CreateActionPlayer) {
    EXPECT_EQ(_actionPlayer->IsStopped(), true);
}
} //  namespace huestream