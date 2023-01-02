#include <huestream/effect/effects/LightSourceEffect.h>
#include <huestream/effect/animation/animations/ConstantAnimation.h>
#include "gtest/gtest.h"
#include "test/huestream/common/TestSerializeBase.h"
#include "huestream/config/ObjectBuilder.h"

namespace huestream {

    class TestLightSourceEffect : public testing::Test, public TestSerializeBase {
    public:
        std::shared_ptr<LightSourceEffect> _effect;
        LightPtr _lightInRadius;
        LightPtr _lightOutRadius;

        virtual void SetUp() {
            Serializable::SetObjectBuilder(std::make_shared<ObjectBuilder>(nullptr));
            _effect = std::make_shared<LightSourceEffect>("Lightsource", 4);
            _effect->SetPositionAnimation(std::make_shared<ConstantAnimation>(-1),
                                          std::make_shared<ConstantAnimation>(1));
            _effect->SetColorAnimation(std::make_shared<ConstantAnimation>(0.1), std::make_shared<ConstantAnimation>(0.2),
                                       std::make_shared<ConstantAnimation>(0.3));
            _effect->SetRadiusAnimation(std::make_shared<ConstantAnimation>(0.5));

            _lightInRadius = std::make_shared<Light>("1", Location(-0.9, 0.9));
            _lightOutRadius = std::make_shared<Light>("2", Location(-0.1, 0.1));
        }

        virtual void TearDown() {
            Serializable::SetObjectBuilder(std::make_shared<ObjectBuilder>(nullptr));
        }

        void assert_colors_matching(std::shared_ptr<LightSourceEffect> effect) {
            auto inRadiusColor = effect->GetColor(_lightInRadius);
            auto outRadiusColor = effect->GetColor(_lightOutRadius);
            ASSERT_DOUBLE_EQ(inRadiusColor.GetR(), 0.1);
            ASSERT_DOUBLE_EQ(inRadiusColor.GetG(), 0.2);
            ASSERT_DOUBLE_EQ(inRadiusColor.GetB(), 0.3);
            ASSERT_NEAR(inRadiusColor.GetAlpha(), 0.92, 0.01);
            ASSERT_DOUBLE_EQ(outRadiusColor.GetAlpha(), 0);
        }
    };

    TEST_F(TestLightSourceEffect, GetColor) {
        assert_colors_matching(_effect);
    }

    TEST_F(TestLightSourceEffect, Serialize) {
        JSONNode node;
        _effect->Serialize(&node);

        std::cout << node.write_formatted() << std::endl;

        auto e = std::make_shared<LightSourceEffect>();
        e->Deserialize(&node);
        assert_colors_matching(e);
    }

    TEST_F(TestLightSourceEffect, Deserialize_UnknownEffectTypeForFactory_AnimationsListHasNoNullAnimations) {

        JSONNode node;
        _effect->Serialize(&node);
        std::cout << node.write_formatted() << std::endl;

        class NullObjectBuilder : public ObjectBuilder {
        protected:
            std::shared_ptr<Serializable> ConstructInstanceOf(std::string type) override {
                return nullptr;
            }
        };

        Serializable::SetObjectBuilder(std::make_shared<NullObjectBuilder>());

        auto e = std::make_shared<LightSourceEffect>();
        e->Deserialize(&node);

        auto animations = e->GetAnimations();
        ASSERT_NE(nullptr, animations);

        for (auto animation : *animations) {
            EXPECT_NE(nullptr, animation);
        }
    }

}  // namespace  huestream
