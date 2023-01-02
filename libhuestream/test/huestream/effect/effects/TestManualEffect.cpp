#include <huestream/effect/effects/ManualEffect.h>
#include "gtest/gtest.h"
#include "test/huestream/_mock/MockTimeManager.h"

namespace huestream {

    class TestManualEffect : public testing::Test {
    protected:
        virtual void SetUp() {
            _timeProvider = std::make_shared<MockTimeManager>();
        }

        virtual void TearDown() {
        }

        void assert_colors_equal(Color color1, Color color2) {
            ASSERT_DOUBLE_EQ(color1.GetR(), color2.GetR());
            ASSERT_DOUBLE_EQ(color1.GetG(), color2.GetG());
            ASSERT_DOUBLE_EQ(color1.GetB(), color2.GetB());
            ASSERT_DOUBLE_EQ(color1.GetAlpha(), color2.GetAlpha());
        }
        std::shared_ptr<MockTimeManager> _timeProvider;
    };

    TEST_F(TestManualEffect, GetColor) {
        auto manualEffect = std::make_shared<ManualEffect>("Some Effect", 0);
        manualEffect->SetIdToColor("3", Color(0.1, 0.2, 0.3));
        auto light1 = std::make_shared<Light>("1", Location(0.5, 0.5));
        auto light3 = std::make_shared<Light>("3", Location(-0.5, 0.5));
        //auto lightRight = std::make_shared<Light>("1", Location(0.5, 0.5));
        auto effect = std::static_pointer_cast<Effect>(manualEffect);

        auto color1 = effect->GetColor(light1);
        auto color3 = effect->GetColor(light3);
        assert_colors_equal(color1, Color(0, 0, 0, 0));
        assert_colors_equal(color3, Color(0.1, 0.2, 0.3, 1.0));
    }

}  // namespace  huestream
