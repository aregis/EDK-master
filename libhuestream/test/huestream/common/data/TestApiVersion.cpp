#include "gtest/gtest.h"
#include <memory>
#include <functional>
#include "huestream/common/data/ApiVersion.h"

using namespace huestream;

class TestApiVersion : public testing::Test {
};

TEST_F(TestApiVersion, InspectAndCompare) {
    ApiVersion v_2_13_5(2, 13, 5);
    ApiVersion v_2_14_3("2.14.3");
    ApiVersion v_10_0_1(10, 0, 1);
    ApiVersion v_10_0_0("10.0.0");
    ApiVersion invalid("invalid");
    ApiVersion v_10_0_1_copy("");
    v_10_0_1_copy.SetVersionString("10.0.1");

    EXPECT_TRUE(v_2_13_5.IsValid());
    EXPECT_TRUE(v_2_14_3.IsValid());
    EXPECT_TRUE(v_10_0_1.IsValid());
    EXPECT_TRUE(v_10_0_0.IsValid());
    EXPECT_TRUE(v_10_0_1_copy.IsValid());
    EXPECT_FALSE(invalid.IsValid());

    EXPECT_EQ(v_2_13_5.GetVersionString(), "2.13.5");
    EXPECT_EQ(v_2_14_3.GetMajor(), 2);
    EXPECT_EQ(v_2_14_3.GetMinor(), 14);
    EXPECT_EQ(v_2_14_3.GetBuild(), 3);

    EXPECT_TRUE(v_2_13_5 < v_2_14_3);
    EXPECT_TRUE(v_2_13_5 <= v_2_14_3);
    EXPECT_TRUE(v_2_13_5 != v_2_14_3);
    EXPECT_FALSE(v_2_13_5 == v_2_14_3);
    EXPECT_FALSE(v_2_13_5 > v_2_14_3);
    EXPECT_FALSE(v_2_13_5 >= v_2_14_3);

    EXPECT_TRUE(v_2_13_5 < v_10_0_0);
    EXPECT_FALSE(v_2_13_5 >= v_10_0_0);
    EXPECT_TRUE(v_10_0_1 > v_10_0_0);
    EXPECT_FALSE(v_10_0_1 <= v_10_0_0);

    EXPECT_TRUE(v_10_0_1 == v_10_0_1_copy);
    EXPECT_TRUE(v_10_0_1 >= v_10_0_1_copy);
    EXPECT_TRUE(v_10_0_1 <= v_10_0_1_copy);
    EXPECT_FALSE(v_10_0_1 != v_10_0_1_copy);
    EXPECT_FALSE(v_10_0_1 < v_10_0_1_copy);
    EXPECT_FALSE(v_10_0_1 > v_10_0_1_copy);
}
