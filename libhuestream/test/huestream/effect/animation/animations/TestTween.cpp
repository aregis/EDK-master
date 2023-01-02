
#include <huestream/effect/animation/animations/TweenAnimation.h>
#include <huestream/effect/animation/animations/ConstantAnimation.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "test/huestream/common/TestSerializeBase.h"

namespace huestream {

class TestTween : public testing::Test, public TestSerializeBase {
 public:
    virtual void SetUp() {}
    virtual void TearDown() {}
};

TEST_F(TestTween, Serialize) {
    auto c = TweenAnimation(10, 20, 30, TweenType::EaseInOutQuad);
    JSONNode node;
    c.Serialize(&node);
    ASSERT_TweenAnimationWithBegin(node, 10, 20, 30, TweenType::EaseInOutQuad);
}

TEST_F(TestTween, SerializeWithoutBegin) {
    auto c = TweenAnimation(20, 30, TweenType::EaseInOutQuad);
    JSONNode node;
    c.Serialize(&node);

    ASSERT_TweenAnimationWithoutBegin(node, 20, 30, TweenType::EaseInOutQuad);
}


TEST_F(TestTween, Deserialize) {
    JSONNode node;
    AddStringAttribute(&node, Serializable::AttributeType, TweenAnimation::type);
    AddDoubleAttribute(&node, TweenAnimation::AttributeBegin, 10);
    AddDoubleAttribute(&node, TweenAnimation::AttributeEnd, 20);
    AddDoubleAttribute(&node, TweenAnimation::AttributeTime, 30);
    AddStringAttribute(&node, TweenAnimation::AttributeTweenType, TweenTypeHelper::ToString(TweenType::Linear));

    auto c = TweenAnimation();
    c.Deserialize(&node);

    ASSERT_TRUE(c.BeginValuePresent());
    ASSERT_EQ(c.GetBegin(), 10);
    ASSERT_EQ(c.GetEnd(), 20);
    ASSERT_EQ(c.GetTime(), 30);
    ASSERT_EQ(c.GetTweenType(), TweenType::Linear);
}

TEST_F(TestTween, DeserializeWithBeginButBeginPresentFalse) {
    JSONNode node;
    AddStringAttribute(&node, Serializable::AttributeType, TweenAnimation::type);
    AddDoubleAttribute(&node, TweenAnimation::AttributeBegin, 10);
    AddDoubleAttribute(&node, TweenAnimation::AttributeEnd, 20);
    AddDoubleAttribute(&node, TweenAnimation::AttributeTime, 30);
    AddStringAttribute(&node, TweenAnimation::AttributeTweenType, TweenTypeHelper::ToString(TweenType::Linear));
    AddBoolAttribute(&node, TweenAnimation::AttributeBeginValuePresent, false);

    auto c = TweenAnimation();
    c.Deserialize(&node);

    ASSERT_FALSE(c.BeginValuePresent());
    ASSERT_EQ(c.GetBegin(), 10);
    ASSERT_EQ(c.GetEnd(), 20);
    ASSERT_EQ(c.GetTime(), 30);
    ASSERT_EQ(c.GetTweenType(), TweenType::Linear);
}

TEST_F(TestTween, DeserializeWithoutBegin) {
    JSONNode node;
    AddStringAttribute(&node, Serializable::AttributeType, TweenAnimation::type);
    AddDoubleAttribute(&node, TweenAnimation::AttributeEnd, 20);
    AddDoubleAttribute(&node, TweenAnimation::AttributeTime, 30);
    AddStringAttribute(&node, TweenAnimation::AttributeTweenType, TweenTypeHelper::ToString(TweenType::Linear));

    auto c = TweenAnimation();
    c.Deserialize(&node);

    ASSERT_FALSE(c.BeginValuePresent());
    ASSERT_EQ(c.GetEnd(), 20);
    ASSERT_EQ(c.GetTime(), 30);
    ASSERT_EQ(c.GetTweenType(), TweenType::Linear);
}

TEST_F(TestTween, GetPositionFromValue) {
    auto c = TweenAnimation(20, 30, TweenType::EaseInOutQuad);

    ASSERT_EQ(c.GetPositionFromValue(10), 15);
    c.SetTweenType(TweenType::Linear);
    ASSERT_EQ(c.GetPositionFromValue(10), 15);
    c.SetTweenType(TweenType::EaseInOutSine);
    ASSERT_EQ(c.GetPositionFromValue(0), 0);
    ASSERT_EQ(c.GetPositionFromValue(10), 0);
}
}  // namespace huestream