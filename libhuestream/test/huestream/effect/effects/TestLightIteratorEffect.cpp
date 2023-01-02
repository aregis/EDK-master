#include <huestream/effect/effects/base/Effect.h>
#include <huestream/effect/animation/animations/ConstantAnimation.h>
#include "gtest/gtest.h"
#include "test/huestream/common/TestSerializeBase.h"
#include "huestream/effect/Mixer.h"
#include "huestream/config/Config.h"
#include "huestream/effect/effects/LightIteratorEffect.h"
#include "huestream/common/time/TimeManager.h"
#include "huestream/common/time/TimeProviderProvider.h"
#include "test/huestream/_stub/StubTimeProvider.h"
#include "huestream/config/ObjectBuilder.h"

using std::make_shared;

namespace huestream {

    class TestLightIteratorEffect : public testing::Test, public TestSerializeBase {
    protected:
        GroupPtr _group;
        std::shared_ptr<StubTimeProvider> _tp;
        using ScopedtimeProviderProvider = support::ScopedProvider<TimeProviderPtr>;
        ScopedtimeProviderProvider _tpScope;
        std::shared_ptr<Player> _player;

        TestLightIteratorEffect()
                : _group(std::make_shared<Group>()),
                  _tp(std::make_shared<StubTimeProvider>()),
                  _tpScope(_tp),
                  _player(std::make_shared<Player>())
        {}

        virtual void SetUp() {
            Serializable::SetObjectBuilder(std::make_shared<ObjectBuilder>(nullptr));
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
    };

    TEST_F(TestLightIteratorEffect, SingleLeftRightTwoLights) {
        auto lightLeft = std::make_shared<Light>("1", Location(-0.5, 0.5));
        auto lightRight = std::make_shared<Light>("2", Location(0.5, 0.5));
        _group->AddLight(lightLeft->GetId(), lightLeft->GetPosition().GetX(), lightLeft->GetPosition().GetY());
        _group->AddLight(lightRight->GetId(), lightRight->GetPosition().GetX(), lightRight->GetPosition().GetY());

        auto iterEffect = std::make_shared<LightIteratorEffect>("Some Effect", 0);
        iterEffect->SetPlayer(_player);
        auto zero = std::make_shared<TweenAnimation>(0, 0, 1000, TweenType::Linear);
        auto one = std::make_shared<TweenAnimation>(1, 1, 1000, TweenType::Linear);
        iterEffect->SetColorAnimation(one, zero, zero);
        iterEffect->SetOpacityAnimation(one);
        iterEffect->SetOffset(800);
        iterEffect->SetOrder(IterationOrderLeftRight);
        iterEffect->SetMode(IterationModeSingle);

        iterEffect->UpdateGroup(_group);
        iterEffect->Enable();
        auto effect = std::static_pointer_cast<Effect>(iterEffect);

        assert_colors_equal(effect->GetColor(lightLeft), Color(1, 0, 0, 1));
        assert_colors_equal(effect->GetColor(lightRight), Color(0, 0, 0, 0));

        _tp->AddMilliseconds(900);
        effect->Render();
        assert_colors_equal(effect->GetColor(lightLeft), Color(1, 0, 0, 1));
        assert_colors_equal(effect->GetColor(lightRight), Color(1, 0, 0, 1));

        _tp->AddMilliseconds(200);
        effect->Render();
        assert_colors_equal(effect->GetColor(lightLeft), Color(0, 0, 0, 0));
        assert_colors_equal(effect->GetColor(lightRight), Color(1, 0, 0, 1));
    }

    TEST_F(TestLightIteratorEffect, BounceCCWThreeLights) {
        auto lightLeft = std::make_shared<Light>("1", Location(-0.5, 0.5));
        auto lightRight = std::make_shared<Light>("2", Location(0.5, 0.5));
        auto lightBack = std::make_shared<Light>("3", Location(0, -0.5));
        _group->AddLight(lightLeft->GetId(), lightLeft->GetPosition().GetX(), lightLeft->GetPosition().GetY());
        _group->AddLight(lightRight->GetId(), lightRight->GetPosition().GetX(), lightRight->GetPosition().GetY());
        _group->AddLight(lightBack->GetId(), lightBack->GetPosition().GetX(), lightBack->GetPosition().GetY());

        auto iterEffect = std::make_shared<LightIteratorEffect>("Some Effect", 0);
        iterEffect->SetPlayer(_player);
        auto zero = std::make_shared<TweenAnimation>(0, 0, 200, TweenType::Linear);
        auto one = std::make_shared<TweenAnimation>(1, 1, 200, TweenType::Linear);
        iterEffect->SetColorAnimation(zero, one, one);
        iterEffect->SetOpacityAnimation(one);
        iterEffect->SetOffset(200);
        iterEffect->SetOrder(IterationOrderClockwise);
        iterEffect->SetInvertOrder(true);
        iterEffect->SetMode(IterationModeBounce);

        iterEffect->UpdateGroup(_group);
        iterEffect->Enable();
        auto effect = std::static_pointer_cast<Effect>(iterEffect);

        assert_colors_equal(effect->GetColor(lightRight), Color(0, 1, 1, 1));
        assert_colors_equal(effect->GetColor(lightLeft), Color(0, 0, 0, 0));
        assert_colors_equal(effect->GetColor(lightBack), Color(0, 0, 0, 0));

        _tp->AddMilliseconds(300);
        effect->Render();
        assert_colors_equal(effect->GetColor(lightRight), Color(0, 0, 0, 0));
        assert_colors_equal(effect->GetColor(lightLeft), Color(0, 1, 1, 1));
        assert_colors_equal(effect->GetColor(lightBack), Color(0, 0, 0, 0));

        _tp->AddMilliseconds(200);
        effect->Render();
        assert_colors_equal(effect->GetColor(lightRight), Color(0, 0, 0, 0));
        assert_colors_equal(effect->GetColor(lightLeft), Color(0, 0, 0, 0));
        assert_colors_equal(effect->GetColor(lightBack), Color(0, 1, 1, 1));

        _tp->AddMilliseconds(200);
        effect->Render();
        assert_colors_equal(effect->GetColor(lightRight), Color(0, 0, 0, 0));
        assert_colors_equal(effect->GetColor(lightLeft), Color(0, 0, 0, 0));
        assert_colors_equal(effect->GetColor(lightBack), Color(0, 1, 1, 1));

        _tp->AddMilliseconds(200);
        effect->Render();
        assert_colors_equal(effect->GetColor(lightRight), Color(0, 0, 0, 0));
        assert_colors_equal(effect->GetColor(lightLeft), Color(0, 1, 1, 1));
        assert_colors_equal(effect->GetColor(lightBack), Color(0, 0, 0, 0));

        _tp->AddMilliseconds(200);
        effect->Render();
        assert_colors_equal(effect->GetColor(lightRight), Color(0, 1, 1, 1));
        assert_colors_equal(effect->GetColor(lightLeft), Color(0, 0, 0, 0));
        assert_colors_equal(effect->GetColor(lightBack), Color(0, 0, 0, 0));

        _tp->AddMilliseconds(400);
        effect->Render();
        assert_colors_equal(effect->GetColor(lightRight), Color(0, 0, 0, 0));
        assert_colors_equal(effect->GetColor(lightLeft), Color(0, 1, 1, 1));
        assert_colors_equal(effect->GetColor(lightBack), Color(0, 0, 0, 0));
    }

    TEST_F(TestLightIteratorEffect, UpdateGroup) {
        auto lightBack = std::make_shared<Light>("2", Location(0.5, -0.5));
        auto lightFront = std::make_shared<Light>("1", Location(0.5, 0.5));

        _group->AddLight(lightBack->GetId(), lightBack->GetPosition().GetX(), lightBack->GetPosition().GetY());

        auto iterEffect = std::make_shared<LightIteratorEffect>("Some Effect", 0);
        iterEffect->SetPlayer(_player);
        auto zero = std::make_shared<TweenAnimation>(0, 0, 1000, TweenType::Linear);
        auto one = std::make_shared<TweenAnimation>(1, 1, 1000, TweenType::Linear);
        iterEffect->SetColorAnimation(one, zero, zero);
        iterEffect->SetOpacityAnimation(one);
        iterEffect->SetOffset(1000);
        iterEffect->SetOrder(IterationOrderFrontBack);
        iterEffect->SetMode(IterationModeCycle);

        iterEffect->UpdateGroup(_group);
        iterEffect->Enable();
        auto effect = std::static_pointer_cast<Effect>(iterEffect);

        _tp->AddMilliseconds(500);
        effect->Render();
        assert_colors_equal(effect->GetColor(lightBack), Color(1, 0, 0, 1));
        assert_colors_equal(effect->GetColor(lightFront), Color(0, 0, 0, 0));

        _tp->AddMilliseconds(1000);
        effect->Render();
        assert_colors_equal(effect->GetColor(lightBack), Color(1, 0, 0, 1));
        assert_colors_equal(effect->GetColor(lightFront), Color(0, 0, 0, 0));
        
        _group->AddLight(lightFront->GetId(), lightFront->GetPosition().GetX(), lightFront->GetPosition().GetY());
        iterEffect->UpdateGroup(_group);

        _tp->AddMilliseconds(1000);
        effect->Render();
        assert_colors_equal(effect->GetColor(lightBack), Color(0, 0, 0, 0));
        assert_colors_equal(effect->GetColor(lightFront), Color(1, 0, 0, 1));
        
    }

    TEST_F(TestLightIteratorEffect, Serialize) {
        auto c = std::make_shared<LightIteratorEffect>("Some Effect", 2);

        c->SetOrder(IterationOrderFrontBack);
        c->SetMode(IterationModeBounce);
        c->SetOffset(500);
        c->SetInvertOrder(true);

        JSONNode node;
        c->Serialize(&node);

        std::string jc = node.write_formatted();
        std::cout << jc << std::endl;

        auto d = std::static_pointer_cast<LightIteratorEffect>(LightIteratorEffect::DeserializeFromJson(&node));
        ASSERT_EQ(d->GetName(), "Some Effect");
        ASSERT_DOUBLE_EQ(d->GetOffset(), 500);
        ASSERT_EQ(d->GetOrder(), IterationOrderFrontBack);
        ASSERT_EQ(d->GetMode(), IterationModeBounce);
        ASSERT_EQ(d->InvertOrder(), true);
    }

}  // namespace  huestream
