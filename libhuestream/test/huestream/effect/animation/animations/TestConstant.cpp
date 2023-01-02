#include <huestream/effect/animation/animations/ConstantAnimation.h>
#include "gtest/gtest.h"
#include "gmock/gmock.h"

namespace huestream { ;

class TestConstant : public testing::Test {
    virtual void SetUp() {
    }

    virtual void TearDown() {
    }
};

TEST_F(TestConstant, Serialize) {
    ConstantAnimation c;
    c.SetValue(112.5);

    JSONNode node;
    c.Serialize(&node);
    ASSERT_EQ(node[Serializable::AttributeType].as_string(), std::string(ConstantAnimation::type));
    ASSERT_TRUE(SerializerHelper::IsAttributeSet(&node, ConstantAnimation::AttributeValue));
    ASSERT_EQ(node[ConstantAnimation::AttributeValue].as_float(), 112.5);
    ASSERT_FALSE(SerializerHelper::IsAttributeSet(&node, ConstantAnimation::AttributeLength));
}

TEST_F(TestConstant, SerializeWithLength) {
    ConstantAnimation c(112.2, 2345);

    JSONNode node;
    c.Serialize(&node);
    ASSERT_EQ(node[Serializable::AttributeType].as_string(), std::string(ConstantAnimation::type));
    ASSERT_EQ(node[ConstantAnimation::AttributeValue].as_float(), 112.2);
    ASSERT_TRUE(SerializerHelper::IsAttributeSet(&node, ConstantAnimation::AttributeLength));
    ASSERT_EQ(node[ConstantAnimation::AttributeLength].as_float(), 2345);
}

TEST_F(TestConstant, Deserialize) {
    ConstantAnimation c(12, 34);

    JSONNode node;
    node.push_back(JSONNode(Serializable::AttributeType, std::string(ConstantAnimation::type)));
    node.push_back(JSONNode(ConstantAnimation::AttributeValue, 112.2));
    c.Deserialize(&node);

    ASSERT_EQ(c.GetValue(), 112.2);
    ASSERT_EQ(c.GetLengthMs(), INF);
}

TEST_F(TestConstant, DeserializeWithLength) {
    ConstantAnimation c(12, 34);

    JSONNode node;
    node.push_back(JSONNode(Serializable::AttributeType, ConstantAnimation::type));
    node.push_back(JSONNode(ConstantAnimation::AttributeValue, 112.2));
    node.push_back(JSONNode(ConstantAnimation::AttributeLength, 5432));
    c.Deserialize(&node);

    ASSERT_EQ(c.GetValue(), 112.2);
    ASSERT_EQ(c.GetLengthMs(), 5432);
}

TEST_F(TestConstant, Clone) {
    ConstantAnimation c(12, 34);
    auto c_clone = c.Clone();
    ASSERT_NE(nullptr,             c_clone);
    ASSERT_EQ(c.GetValue(),       c_clone->GetValue());
    ASSERT_EQ(c.GetLengthMs(),    c_clone->GetLengthMs());
    ASSERT_EQ(c.GetTypeName(),    c_clone->GetTypeName());
    ASSERT_EQ(c.GetTotalLength(), c_clone->GetTotalLength());
}

}  // namespace huestream