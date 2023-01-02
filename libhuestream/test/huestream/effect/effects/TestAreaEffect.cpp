#include <huestream/effect/effects/AreaEffect.h>
#include <huestream/effect/effects/base/Effect.h>
#include <huestream/effect/animation/animations/ConstantAnimation.h>
#include <huestream/common/data/Area.h>
#include "gtest/gtest.h"
#include "test/huestream/common/TestSerializeBase.h"
#include "test/huestream/_mock/MockTimeManager.h"
#include "huestream/config/ObjectBuilder.h"

namespace huestream {

    class TestAreaEffect : public testing::Test, public TestSerializeBase {
    protected:
        virtual void SetUp() {
            Serializable::SetObjectBuilder(std::make_shared<ObjectBuilder>(nullptr));
            _timeProvider = std::make_shared<MockTimeManager>();
        }

        virtual void TearDown() {
            Serializable::SetObjectBuilder(std::make_shared<ObjectBuilder>(nullptr));
        }

        void assert_colors_equal(Color color1, Color color2) {
            ASSERT_DOUBLE_EQ(color1.GetR(), color2.GetR());
            ASSERT_DOUBLE_EQ(color1.GetG(), color2.GetG());
            ASSERT_DOUBLE_EQ(color1.GetB(), color2.GetB());
            ASSERT_DOUBLE_EQ(color1.GetAlpha(), color2.GetAlpha());
        }
        std::shared_ptr<MockTimeManager> _timeProvider;
    };

    TEST_F(TestAreaEffect, GetColor) {
        auto areaEffect = std::make_shared<AreaEffect>("Some Effect", 0);
        areaEffect->SetColorAnimation(std::make_shared<ConstantAnimation>(0.1), std::make_shared<ConstantAnimation>(0.2),
                                      std::make_shared<ConstantAnimation>(0.4));
        auto lightLeft = std::make_shared<Light>("1", Location(-0.5, 0.5));
        auto lightRight = std::make_shared<Light>("2", Location(0.5, 0.5));
        auto effect = std::static_pointer_cast<Effect>(areaEffect);

        auto colorLeft = effect->GetColor(lightLeft);
        auto colorRight = effect->GetColor(lightRight);
        assert_colors_equal(colorLeft, Color(0, 0, 0, 0));
        assert_colors_equal(colorRight, Color(0, 0, 0, 0));

        areaEffect->AddArea(Area::LeftHalf);
        colorLeft = effect->GetColor(lightLeft);
        colorRight = effect->GetColor(lightRight);
        assert_colors_equal(colorLeft, Color(0.1, 0.2, 0.4, 1.0));
        assert_colors_equal(colorRight, Color(0, 0, 0, 0));

        areaEffect->AddArea(Area::RightHalf);
        colorLeft = effect->GetColor(lightLeft);
        colorRight = effect->GetColor(lightRight);
        assert_colors_equal(colorLeft, Color(0.1, 0.2, 0.4, 1.0));
        assert_colors_equal(colorRight, Color(0.1, 0.2, 0.4, 1.0));

        areaEffect->SetArea(Area::RightHalf);
        colorLeft = effect->GetColor(lightLeft);
        colorRight = effect->GetColor(lightRight);
        assert_colors_equal(colorLeft, Color(0, 0, 0, 0));
        assert_colors_equal(colorRight, Color(0.1, 0.2, 0.4, 1.0));

        areaEffect->SetOpacityAnimation(std::make_shared<ConstantAnimation>(0.6));
        colorLeft = effect->GetColor(lightLeft);
        colorRight = effect->GetColor(lightRight);
        assert_colors_equal(colorLeft, Color(0, 0, 0, 0));
        assert_colors_equal(colorRight, Color(0.1, 0.2, 0.4, 0.6));

        ASSERT_TRUE(areaEffect->GetAnimations()->size() == 4);
        areaEffect->SetIntensityAnimation(std::make_shared<ConstantAnimation>(0.8));
        ASSERT_TRUE(areaEffect->GetAnimations()->size() == 5);

        colorLeft = effect->GetColor(lightLeft);
        colorRight = effect->GetColor(lightRight);
        assert_colors_equal(colorLeft, Color(0, 0, 0, 0));
        assert_colors_equal(colorRight, Color(0.2, 0.4, 0.8, 0.6));
    }

    TEST_F(TestAreaEffect, Serialize) {
        auto c = std::make_shared<AreaEffect>("Some Effect", 2);
        c->SetColorAnimation(std::make_shared<ConstantAnimation>(0.1), std::make_shared<ConstantAnimation>(0.2),
                             std::make_shared<ConstantAnimation>(0.3));
        c->SetArea(Area::BackHalf);

        JSONNode node;
        c->Serialize(&node);
        std::string jc = node.write_formatted();

        std::cout << jc << std::endl;

        auto d = std::static_pointer_cast<AreaEffect>(AreaEffect::DeserializeFromJson(&node));
        //ASSERT_EQ(d->GetLayer(), 2);
        ASSERT_EQ(d->GetName(), "Some Effect");

        auto effect = std::static_pointer_cast<Effect>(d);
        auto lightFront = std::make_shared<Light>("1", Location(0, 0.5));
        auto lightBack = std::make_shared<Light>("2", Location(0, -0.5));
        auto colorFront = effect->GetColor(lightFront);
        auto colorBack = effect->GetColor(lightBack);
        assert_colors_equal(colorFront, Color(0, 0, 0, 0));
        assert_colors_equal(colorBack, Color(0.1, 0.2, 0.3, 1));
    }

}  // namespace  huestream
