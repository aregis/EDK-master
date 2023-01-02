#include "huestream/effect/effects/MultiChannelEffect.h"
#include <huestream/effect/effects/base/Effect.h>
#include <huestream/effect/animation/animations/ConstantAnimation.h>
#include <huestream/config/ObjectBuilder.h>
#include "gtest/gtest.h"
#include "test/huestream/common/TestSerializeBase.h"
#include "huestream/effect/Mixer.h"
#include "huestream/config/Config.h"
#include "test/huestream/_stub/StubTimeProvider.h"
#include "test/huestream/_mock/MockTimeManager.h"

namespace huestream {

    class TestMultiChannelEffect : public testing::Test, public TestSerializeBase {
    protected:
        GroupPtr _group;
        ChannelPtr _redLeftChannel;
        ChannelPtr _whiteMiddleChannel;
        ChannelPtr _blueRightChannel;
        std::shared_ptr<MockTimeManager> _timeProvider;

        virtual void SetUp() {
            _group = std::make_shared<Group>();
            _timeProvider = std::make_shared<MockTimeManager>();
            Serializable::SetObjectBuilder(std::make_shared<ObjectBuilder>(std::make_shared<BridgeSettings>()));

            auto zero = std::make_shared<ConstantAnimation>(0);
            auto one = std::make_shared<ConstantAnimation>(1);
            _redLeftChannel = std::make_shared<Channel>(Location(-0.5, 0.1), one, zero, zero, one);
            _whiteMiddleChannel = std::make_shared<Channel>(Location(0, 0), one, one, one, one);
            _blueRightChannel = std::make_shared<Channel>(Location(0.5, -0.1), zero, zero, one, one);
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

    TEST_F(TestMultiChannelEffect, ThreeChannelsTwoLights) {
        auto lightLeft = std::make_shared<Light>("1", Location(-0.5, 0.5));
        auto lightRight = std::make_shared<Light>("2", Location(0.5, 0.5));
        _group->AddLight(lightLeft->GetId(), lightLeft->GetPosition().GetX(), lightLeft->GetPosition().GetY());
        _group->AddLight(lightRight->GetId(), lightRight->GetPosition().GetX(), lightRight->GetPosition().GetY());

        auto channelEffect = std::make_shared<MultiChannelEffect>();
        channelEffect->UpdateGroup(_group);
        channelEffect->AddChannel(_redLeftChannel);
        channelEffect->AddChannel(_whiteMiddleChannel);
        channelEffect->AddChannel(_blueRightChannel);
        auto effect = std::static_pointer_cast<Effect>(channelEffect);

        assert_colors_equal(effect->GetColor(lightLeft), Color(1, 0, 0, 1));
        assert_colors_equal(effect->GetColor(lightRight), Color(0, 0, 1, 1));
    }

    TEST_F(TestMultiChannelEffect, TwoChannelsThreeLights) {
        auto lightRightTop = std::make_shared<Light>("1", Location(0.5, 0.5));
        auto lightRightMiddle = std::make_shared<Light>("2", Location(0.5, 0));
        auto lightRightBottom = std::make_shared<Light>("3", Location(0.5, -0.5));
        _group->AddLight(lightRightTop->GetId(), lightRightTop->GetPosition().GetX(),
                         lightRightTop->GetPosition().GetY());
        _group->AddLight(lightRightMiddle->GetId(), lightRightMiddle->GetPosition().GetX(),
                         lightRightMiddle->GetPosition().GetY());
        _group->AddLight(lightRightBottom->GetId(), lightRightBottom->GetPosition().GetX(),
                         lightRightBottom->GetPosition().GetY());

        auto channelEffect = std::make_shared<MultiChannelEffect>();
        channelEffect->AddChannel(_redLeftChannel);
        channelEffect->AddChannel(_whiteMiddleChannel);
        channelEffect->UpdateGroup(_group);
        auto effect = std::static_pointer_cast<Effect>(channelEffect);

        assert_colors_equal(effect->GetColor(lightRightTop), Color(1, 0, 0, 1));
        assert_colors_equal(effect->GetColor(lightRightMiddle), Color(1, 1, 1, 1));
        assert_colors_equal(effect->GetColor(lightRightBottom), Color(1, 1, 1, 1));
    }

    TEST_F(TestMultiChannelEffect, Serialize) {
        auto c = std::make_shared<MultiChannelEffect>("Some effect");
        auto zero = std::make_shared<ConstantAnimation>(0);
        auto one = std::make_shared<ConstantAnimation>(1);
        c->AddChannel(std::make_shared<Channel>(Location(0, 0), one, zero, zero, one));
        c->AddChannel(std::make_shared<Channel>(Location(-1, 0.5), zero, one, zero, one));

        JSONNode node;
        c->Serialize(&node);
        std::string jc = node.write_formatted();

        std::cout << jc << std::endl;

        auto lightFront = std::make_shared<Light>("1", Location(-0.5, 0.5));
        auto lightBack = std::make_shared<Light>("2", Location(0, -0.5));
        _group->AddLight(lightFront->GetId(), lightFront->GetPosition().GetX(), lightFront->GetPosition().GetY());
        _group->AddLight(lightBack->GetId(), lightBack->GetPosition().GetX(), lightBack->GetPosition().GetY());

        auto d = std::static_pointer_cast<MultiChannelEffect>(MultiChannelEffect::DeserializeFromJson(&node));
        //ASSERT_EQ(d->GetLayer(), 2);
        ASSERT_EQ(d->GetName(), "Some effect");

        auto effect = std::static_pointer_cast<Effect>(d);
        effect->UpdateGroup(_group);

        assert_colors_equal(effect->GetColor(lightFront), Color(0, 1, 0, 1));
        assert_colors_equal(effect->GetColor(lightBack), Color(1, 0, 0, 1));
    }

}  // namespace  huestream
