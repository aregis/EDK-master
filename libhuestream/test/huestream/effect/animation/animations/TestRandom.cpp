#include <huestream/effect/animation/animations/RandomAnimation.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "test/huestream/common/TestSerializeBase.h"

namespace huestream {

    class TestRandom : public testing::Test, public TestSerializeBase {
     public:
        virtual void SetUp() {}
        virtual void TearDown() {}
    };

    TEST_F(TestRandom, Serialize) {
        auto c = RandomAnimation(10, 20, 30, 40, TweenType::EaseInOutQuad, 50);
        JSONNode node;
        c.Serialize(&node);
        ASSERT_RandomAnimationWithLength(node, 10, 20, 30, 40, TweenType::EaseInOutQuad, 50);
    }

    TEST_F(TestRandom, SerializeWithoutLength) {
        auto c = RandomAnimation(10, 20, 30, 40, TweenType::EaseInOutQuad, INF);
        JSONNode node;
        c.Serialize(&node);
        ASSERT_RandomAnimationWithoutLength(node, 10, 20, 30, 40, TweenType::EaseInOutQuad);
    }

    TEST_F(TestRandom, Deserialize) {
        JSONNode node;
        AddStringAttribute(&node, Serializable::AttributeType, RandomAnimation::type);
        AddDoubleAttribute(&node, RandomAnimation::AttributeMinValue, 10);
        AddDoubleAttribute(&node, RandomAnimation::AttributeMaxValue, 20);
        AddDoubleAttribute(&node, RandomAnimation::AttributeMinInterval, 30);
        AddDoubleAttribute(&node, RandomAnimation::AttributeMaxInterval, 40);
        AddStringAttribute(&node, RandomAnimation::AttributeTweenType, TweenTypeHelper::ToString(TweenType::Linear));
        AddDoubleAttribute(&node, RandomAnimation::AttributeLength, 50);

        auto c = RandomAnimation();
        c.Deserialize(&node);

        ASSERT_EQ(c.GetMinValue(), 10);
        ASSERT_EQ(c.GetMaxValue(), 20);
        ASSERT_EQ(c.GetMinInterval(), 30);
        ASSERT_EQ(c.GetMaxInterval(), 40);
        ASSERT_EQ(c.GetTweenType(), TweenType::Linear);
        ASSERT_EQ(c.GetLength(), 50);
    }

    TEST_F(TestRandom, DeserializeWithoutLength) {
        JSONNode node;
        AddStringAttribute(&node, Serializable::AttributeType, RandomAnimation::type);
        AddDoubleAttribute(&node, RandomAnimation::AttributeMinValue, 10);
        AddDoubleAttribute(&node, RandomAnimation::AttributeMaxValue, 20);
        AddDoubleAttribute(&node, RandomAnimation::AttributeMinInterval, 30);
        AddDoubleAttribute(&node, RandomAnimation::AttributeMaxInterval, 40);
        AddStringAttribute(&node, RandomAnimation::AttributeTweenType, TweenTypeHelper::ToString(TweenType::Linear));

        auto c = RandomAnimation();
        c.Deserialize(&node);

        ASSERT_EQ(c.GetMinValue(), 10);
        ASSERT_EQ(c.GetMaxValue(), 20);
        ASSERT_EQ(c.GetMinInterval(), 30);
        ASSERT_EQ(c.GetMaxInterval(), 40);
        ASSERT_EQ(c.GetTweenType(), TweenType::Linear);
        ASSERT_EQ(c.GetLength(), INF);
    }
}  // namespace huestream