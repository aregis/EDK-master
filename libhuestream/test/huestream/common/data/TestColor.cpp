#include "gtest/gtest.h"
#include <memory>
#include <functional>
#include "huestream/common/data/Color.h"

using namespace huestream;

class TestColor : public testing::Test {
protected:
    virtual void SetUp() {
    }

    virtual void TearDown() {
    }

    virtual void AssertColor(Color c, Color d) {
        EXPECT_NEAR(d.GetR(), c.GetR(), .001);
        EXPECT_NEAR(d.GetG(), c.GetG(), .001);
        EXPECT_NEAR(d.GetB(), c.GetB(), .001);
    }

    virtual void SetBrightnessAndAssert(Color in, double bri, Color expected) {
        ASSERT_DOUBLE_EQ(bri, expected.GetCurrentBrightness()); //validate expected value
        in.ApplyBrightness(bri);
        ASSERT_DOUBLE_EQ(bri, in.GetCurrentBrightness());
        AssertColor(in, expected);
    }
};

TEST_F(TestColor, GetBrightness) {
    ASSERT_EQ(1.0, Color(1.0, 0.0, 0.0).GetCurrentBrightness());
    ASSERT_EQ(1.0, Color(1.0, 1.0, 0.0).GetCurrentBrightness());
    ASSERT_EQ(1.0, Color(1.0, 0.0, 1.0).GetCurrentBrightness());
    ASSERT_EQ(1.0, Color(1.0, 0.5, 1.0).GetCurrentBrightness());
    ASSERT_EQ(0.5, Color(0.5, 0.0, 0.0).GetCurrentBrightness());
    ASSERT_EQ(0.5, Color(0.5, 0.1, 0.1).GetCurrentBrightness());

    double xy[2] = {0.5018, 0.4152};
		ASSERT_NEAR(0.06186248, Color(xy, 0.567).GetCurrentBrightness(), 0.001);
}

TEST_F(TestColor, SetBrightness) {
    SetBrightnessAndAssert(Color(1.0, 0.0, 0.0), 0.50, Color(0.5, 0.0, 0.0));
    SetBrightnessAndAssert(Color(1.0, 0.0, 1.0), 0.50, Color(0.5, 0.0, 0.5));
    SetBrightnessAndAssert(Color(0.5, 0.0, 0.0), 1.00, Color(1.0, 0.0, 0.0));
    SetBrightnessAndAssert(Color(0.7, 0.9, 0.4), 0.45, Color(0.35, 0.45, 0.2));
    SetBrightnessAndAssert(Color(0.7, 0.9, 0.4), 1.00, Color(0.777, 1.0, 0.444));

    double xy[2] = {0.5018, 0.4152};
    SetBrightnessAndAssert(Color(xy, 0.567), 1.00, Color(1.0, 0.33246053, 0.03478684));
}

TEST_F(TestColor, Clamp) {
    Color color = Color(-0.5, 0.5, 3.0);
    ASSERT_DOUBLE_EQ(color.GetR(), -0.5);
    ASSERT_DOUBLE_EQ(color.GetG(), 0.5);
    ASSERT_DOUBLE_EQ(color.GetB(), 3);

    color.Clamp();
    ASSERT_DOUBLE_EQ(color.GetR(), 0);
    ASSERT_DOUBLE_EQ(color.GetG(), 0.5);
    ASSERT_DOUBLE_EQ(color.GetB(), 1);
}

TEST_F(TestColor, GetRGBA) {
    Color color(1.0, 0.0, 0.0);
    std::vector<double> rgba = color.GetRGBA();
    ASSERT_EQ(4, rgba.size());
    ASSERT_EQ(color.GetAlpha(), rgba[3]);
    ASSERT_EQ(color.GetB(),     rgba[2]);
    ASSERT_EQ(color.GetG(),     rgba[1]);
    ASSERT_EQ(color.GetR(),     rgba[0]);
}



typedef std::function<void(Color)> callback;

void bla(std::function<void(Color)> f) {
    Color c(0.5, 0.4, 0.3);
    //  lock
    f(c);
    //  unlock
}

#define HUECTX  bla([](Color c)

#define HUECTX2(aap)  bla([](Color c) aap);


TEST_F(TestColor, func) {

//    bla([](Color c){
//        ASSERT_EQ(0.5, c.GetR());
//        ASSERT_EQ(0.4, c.GetG());
//        ASSERT_EQ(0.3, c.GetB());
//    });

    HUECTX2({
                ASSERT_EQ(0.5, c.GetR());
                ASSERT_EQ(0.4, c.GetG());
                ASSERT_EQ(0.3, c.GetB());
            });

}