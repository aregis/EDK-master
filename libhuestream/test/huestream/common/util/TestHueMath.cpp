#include "gtest/gtest.h"
#include <huestream/common/util/HueMath.h>
#include <huestream/common/util/Rand.h>

#include <memory>

using std::tuple;
using std::make_tuple;
using namespace testing;

typedef double math_alg(double t, double b, double e, double d);

class TestHueMath : public testing::TestWithParam<tuple<math_alg*, double>>  {

};

INSTANTIATE_TEST_CASE_P(ease_math_valid_value, TestHueMath, Values(
    make_tuple(HueMath::easeInOutCirc,  -0.916),
    make_tuple(HueMath::easeOutCirc,    -0.959),
    make_tuple(HueMath::easeInCirc,      0.199),
    make_tuple(HueMath::easeInOutExpo,  -0.984),
    make_tuple(HueMath::easeOutExpo,    -0.992),
    make_tuple(HueMath::easeInOutSine,  -0.809),
    make_tuple(HueMath::easeOutSine,    -0.902),
    make_tuple(HueMath::easeInSine,     -0.381),
    make_tuple(HueMath::easeInOutQuint, -0.989),
    make_tuple(HueMath::easeOutQuint,   -0.999),
    make_tuple(HueMath::easeInQuint,     0.344),
    make_tuple(HueMath::easeInOutQuart, -0.974),
    make_tuple(HueMath::easeOutQuart,   -0.996),
    make_tuple(HueMath::easeInQuart,     0.180),
    make_tuple(HueMath::easeInOutCubic, -0.935),
    make_tuple(HueMath::easeOutCubic,   -0.984),
    make_tuple(HueMath::easeInCubic,    -0.024),
    make_tuple(HueMath::easeInOutQuad,  -0.840),
    make_tuple(HueMath::easeOutQuad,    -0.919),
    make_tuple(HueMath::easeInQuad,     -0.280),
    make_tuple(HueMath::linearTween,    -0.600)
));

TEST_P(TestHueMath, ease_math_valid_value){
    const auto &test_arguments = GetParam();
    EXPECT_EQ(-1, (std::get<0>(test_arguments))(6, 1, -1, 5));
    EXPECT_NEAR(std::get<1>(test_arguments), (std::get<0>(test_arguments))(4, 1, -1, 5), 0.00100000005);
}

TEST_F(TestHueMath, IsAngleBetween) {
    double target = 90;
    double angel_1 = 45;
    double angel_2 = 60;
    double angel_3 = 180;

    EXPECT_TRUE(HueMath::isAngleBetween(target, angel_1, angel_3));
    EXPECT_TRUE(HueMath::isAngleBetween(target, angel_3, angel_1));
    EXPECT_FALSE(HueMath::isAngleBetween(target, angel_1, angel_2));
}

TEST_F(TestHueMath, GetDiffDegrees) {
    EXPECT_EQ(90,  HueMath::getDiffDegrees(180, 90));
    EXPECT_EQ(90,  HueMath::getDiffDegrees(90,  180));
    EXPECT_EQ(0,   HueMath::getDiffDegrees(180, 180));
    EXPECT_EQ(270, HueMath::getDiffDegrees(90,  -180));
    EXPECT_EQ(45,  HueMath::getDiffDegrees(0,   45));
}
