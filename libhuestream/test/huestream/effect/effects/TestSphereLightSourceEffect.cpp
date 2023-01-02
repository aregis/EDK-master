#include <huestream/effect/effects/SphereLightSourceEffect.h>
#include <huestream/effect/animation/animations/ConstantAnimation.h>
#include "gtest/gtest.h"
#include "test/huestream/common/TestSerializeBase.h"
#include "huestream/config/ObjectBuilder.h"

namespace huestream {

    class TestSphereLightSourceEffect : public testing::Test, public TestSerializeBase {
    public:
        std::shared_ptr<SphereLightSourceEffect> _effect;
        LightPtr _lightInRadius;
        LightPtr _lightOutRadius;

        virtual void SetUp() {
            Serializable::SetObjectBuilder(std::make_shared<ObjectBuilder>(nullptr));
            _effect = std::make_shared<SphereLightSourceEffect>("SphereLightsource", 4);
            _effect->SetPositionAnimation(std::make_shared<ConstantAnimation>(-1),
                                          std::make_shared<ConstantAnimation>(1),
                                          std::make_shared<ConstantAnimation>(1));
            _effect->SetColorAnimation(std::make_shared<ConstantAnimation>(0.1), std::make_shared<ConstantAnimation>(0.2),
                                       std::make_shared<ConstantAnimation>(0.3));
            _effect->SetRadiusAnimation(std::make_shared<ConstantAnimation>(0.5));

            _lightInRadius = std::make_shared<Light>("1", Location(-0.9, 0.9, 0.9));
            _lightOutRadius = std::make_shared<Light>("2", Location(-0.1, 0.1, 0.1));
        }

        virtual void TearDown() {
            Serializable::SetObjectBuilder(std::make_shared<ObjectBuilder>(nullptr));
        }

        void assert_colors_matching(std::shared_ptr<SphereLightSourceEffect> effect) {
            auto inRadiusColor = effect->GetColor(_lightInRadius);
            auto outRadiusColor = effect->GetColor(_lightOutRadius);
            ASSERT_DOUBLE_EQ(inRadiusColor.GetR(), 0.1);
            ASSERT_DOUBLE_EQ(inRadiusColor.GetG(), 0.2);
            ASSERT_DOUBLE_EQ(inRadiusColor.GetB(), 0.3);
            ASSERT_NEAR(inRadiusColor.GetAlpha(), 0.88, 0.01);
            ASSERT_DOUBLE_EQ(outRadiusColor.GetAlpha(), 0);
        }
    };

    TEST_F(TestSphereLightSourceEffect, GetColor) {
        assert_colors_matching(_effect);
    }

    TEST_F(TestSphereLightSourceEffect, Serialize) {
        JSONNode node;
        _effect->Serialize(&node);

        std::cout << node.write_formatted() << std::endl;

        auto e = std::make_shared<SphereLightSourceEffect>();
        e->Deserialize(&node);
        assert_colors_matching(e);
    }

}  // namespace  huestream
