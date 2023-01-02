#include <huestream/effect/animation/data/CurveOptions.h>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "test/huestream/common/TestSerializeBase.h"

namespace huestream { ;

    class TestCurveOptions : public testing::Test, public TestSerializeBase {
        virtual void SetUp() {

        }

        virtual void TearDown() {
        }
    };

    TEST_F(TestCurveOptions, Serialize) {
        auto c = CurveOptions(3, Nullable<double>(100), Nullable<double>(200));

        JSONNode node;
        c.Serialize(&node);
        ASSERT_CurveOptionsAllAttributesSet(node, 3, 100, 200);

    }

    TEST_F(TestCurveOptions, SerializeDefault) {
        auto c = CurveOptions();

        JSONNode node;
        c.Serialize(&node);
        ASSERT_AttributeIsSetAndDoubleValue(node, CurveOptions::AttributeMultiplyFactor, 1);
        ASSERT_FALSE(SerializerHelper::IsAttributeSet(&node, CurveOptions::AttributeClipMin));
        ASSERT_FALSE(SerializerHelper::IsAttributeSet(&node, CurveOptions::AttributeClipMin));
    }

    TEST_F(TestCurveOptions, Deserialize) {
        JSONNode node;
        AddStringAttribute(&node, Serializable::AttributeType, CurveOptions::type);
        AddDoubleAttribute(&node, CurveOptions::AttributeMultiplyFactor, 3);
        AddDoubleAttribute(&node, CurveOptions::AttributeClipMin, 100);
        AddDoubleAttribute(&node, CurveOptions::AttributeClipMax, 200);

        auto c = CurveOptions();
        c.Deserialize(&node);

        ASSERT_EQ(3, c.GetMultiplyFactor());
        ASSERT_EQ(100, c.GetClipMin().get_value());
        ASSERT_EQ(200, c.GetClipMax().get_value());
    }

    TEST_F(TestCurveOptions, DeserializeDefault) {
        JSONNode node;
        AddStringAttribute(&node, Serializable::AttributeType, CurveOptions::type);

        auto c = CurveOptions();
        c.Deserialize(&node);

        ASSERT_EQ(1, c.GetMultiplyFactor());
        ASSERT_FALSE(c.GetClipMin().has_value());
        ASSERT_FALSE(c.GetClipMax().has_value());
    }


}  // namespace huestream