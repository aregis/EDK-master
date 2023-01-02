#include <huestream/effect/animation/data/Point.h>
#include <huestream/effect/animation/data/Vector.h>
#include "gtest/gtest.h"

#include "huestream/common/data/Color.h"

using namespace huestream;

class TestVector : public testing::Test {
protected:
    virtual void SetUp() {
    }

    virtual void TearDown() {
    }
};

TEST_F(TestVector, ResultIsEqualToLength) {
    EXPECT_EQ(8, Vector(Point(0, 0), Point(8, 0)).GetLength());
    EXPECT_EQ(8, Vector(Point(8, 0), Point(0, 0)).GetLength());
    EXPECT_EQ(8, Vector(Point(0, 0), Point(0, 8)).GetLength());
    EXPECT_EQ(8, Vector(Point(0, 8), Point(0, 0)).GetLength());

    EXPECT_EQ(8, Vector(Point(1, 0), Point(9, 0)).GetLength());
    EXPECT_EQ(8, Vector(Point(9, 0), Point(1, 0)).GetLength());
    EXPECT_NEAR(5.65685, Vector(Point(0, 0), Point(4, 4)).GetLength(), 0.0001);
    EXPECT_NEAR(5.65685, Vector(Point(4, 4), Point(0, 0)).GetLength(), 0.0001);
}

TEST_F(TestVector, ResultIsEqualToQuadrant) {
    EXPECT_EQ(0, Vector(Point(0, 0), Point(1, 1)).GetQuadrant());
    EXPECT_EQ(1, Vector(Point(0, 0), Point(-1, 1)).GetQuadrant());
    EXPECT_EQ(2, Vector(Point(0, 0), Point(-1, -1)).GetQuadrant());
    EXPECT_EQ(3, Vector(Point(0, 0), Point(1, -1)).GetQuadrant());

    EXPECT_EQ(2, Vector(Point(1, 1), Point(0, 0)).GetQuadrant());
    EXPECT_EQ(1, Vector(Point(1, -1), Point(0, 0)).GetQuadrant());
    EXPECT_EQ(0, Vector(Point(-1, -1), Point(0, 0)).GetQuadrant());
    EXPECT_EQ(3, Vector(Point(-1, 1), Point(0, 0)).GetQuadrant());
}

TEST_F(TestVector, ResultIsEqualToAngle) {
    EXPECT_NEAR(0, Vector(Point(0, 0), Point(1, 0)).GetAngle(), 0.0001);
    EXPECT_NEAR(45, Vector(Point(0, 0), Point(1, 1)).GetAngle(), 0.0001);
    EXPECT_NEAR(90, Vector(Point(0, 0), Point(0, 1)).GetAngle(), 0.0001);
    EXPECT_NEAR(135, Vector(Point(0, 0), Point(-1, 1)).GetAngle(), 0.0001);
    EXPECT_NEAR(180, Vector(Point(0, 0), Point(-1, 0)).GetAngle(), 0.0001);
    EXPECT_NEAR(225, Vector(Point(0, 0), Point(-1, -1)).GetAngle(), 0.0001);
    EXPECT_NEAR(270, Vector(Point(0, 0), Point(0, -1)).GetAngle(), 0.0001);
    EXPECT_NEAR(315, Vector(Point(0, 0), Point(1, -1)).GetAngle(), 0.0001);

    EXPECT_NEAR(0, Vector(Point(-1, 0), Point(0, 0)).GetAngle(), 0.0001);
    EXPECT_NEAR(45, Vector(Point(-1, -1), Point(0, 0)).GetAngle(), 0.0001);
    EXPECT_NEAR(90, Vector(Point(0, -1), Point(0, 0)).GetAngle(), 0.0001);
    EXPECT_NEAR(135, Vector(Point(1, -1), Point(0, 0)).GetAngle(), 0.0001);
    EXPECT_NEAR(180, Vector(Point(1, 0), Point(0, 0)).GetAngle(), 0.0001);
    EXPECT_NEAR(225, Vector(Point(1, 1), Point(0, 0)).GetAngle(), 0.0001);
    EXPECT_NEAR(270, Vector(Point(0, 1), Point(0, 0)).GetAngle(), 0.0001);
    EXPECT_NEAR(315, Vector(Point(-1, 1), Point(0, 0)).GetAngle(), 0.0001);
}

TEST_F(TestVector, ResultIsEqualToXY) {
    Vector v_1 = Vector(Location(), Location(1, 0));
    Vector v_2 = Vector(Point(), Point(1, 1));

    EXPECT_EQ(1, v_1.get_x());
    EXPECT_EQ(0, v_1.get_y());

    EXPECT_EQ(1, v_2.get_x());
    EXPECT_EQ(1, v_2.get_y());
}
