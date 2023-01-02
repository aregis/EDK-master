
#include "test/huestream/common/TestSerializeBase.h"
#include "test/huestream/_mock/MockTimeManager.h"
#include <huestream/effect/effects/ExplosionEffect.h>
#include <huestream/effect/animation/animations/ConstantAnimation.h>
#include <gtest/gtest.h>
#include <huestream/effect/Mixer.h>
#include "huestream/common/time/TimeProviderProvider.h"

using ::testing::Return;

namespace huestream {



    class TestExplosionEffect : public testing::Test, public TestSerializeBase {
    public:
        std::shared_ptr<ExplosionEffect> _effect;
        LightPtr _lightInRadius;
        LightPtr _lightOutRadius;
        std::shared_ptr<MockTimeManager> _mockTimeManager;
        ScopedTimeProviderProvider _timeProvider;
        long long int _currentTime;
        std::shared_ptr<Mixer> _engine;

        TestExplosionEffect() :
                _effect(std::make_shared<ExplosionEffect>("ExplosionEffect", 4)),
                _lightInRadius(std::make_shared<Light>("1", Location(0.0, 0.0))),
                _lightOutRadius(std::make_shared<Light>("2", Location(1.0, 0.0))),
                _mockTimeManager(std::make_shared<MockTimeManager>()),
                _timeProvider(_mockTimeManager),
                _currentTime(0),
                _engine(std::make_shared<Mixer>()) {
        }

        virtual void SetUp() {
            IncreaseTime(0);
        }

        void IncreaseTime(int ms) {
            _currentTime += ms;
            EXPECT_CALL(*_mockTimeManager, Now()).WillRepeatedly(Return(_currentTime));
        }

        virtual void TearDown() {}

        bool Assert_Color(const Color &actualColor, const Color &expectedColor) const {
            EXPECT_DOUBLE_EQ(actualColor.GetR(), expectedColor.GetR());
            EXPECT_DOUBLE_EQ(actualColor.GetG(), expectedColor.GetG());
            EXPECT_DOUBLE_EQ(actualColor.GetB(), expectedColor.GetB());
            EXPECT_DOUBLE_EQ(actualColor.GetAlpha(), expectedColor.GetAlpha());
            return !::testing::Test::HasFailure();
        }

    };

    TEST_F(TestExplosionEffect, IntensityAtTheCenterOfTheExplosion) {
        double duration = 1000;
        double maxIntensityTime = 500;
        double maxRadiusTime = 500;
        double radius = .5;

        _effect->PrepareEffect(Color(0.5, 0, 0), Location(0, 0), duration, radius, maxIntensityTime, maxRadiusTime);
        _effect->Enable();

        //At t = 0 (beginning) no contribution
        ASSERT_TRUE(Assert_Color(_effect->GetColor(_lightInRadius), Color(0,0,0,0)));

        //At t = half duration intensity is half)
        IncreaseTime((int)maxIntensityTime / 2);
        _effect->Render();
        ASSERT_TRUE(Assert_Color(_effect->GetColor(_lightInRadius), Color(0.25,0,0,1)));

        //At the peak of the explosion the color reaches the intensity of the set color
        IncreaseTime((int)maxIntensityTime / 2);
        _effect->Render();
        ASSERT_TRUE(Assert_Color(_effect->GetColor(_lightInRadius), Color(0.5,0,0,1)));

        IncreaseTime((int)(duration - maxIntensityTime) / 4);
        _effect->Render();
        ASSERT_TRUE(Assert_Color(_effect->GetColor(_lightInRadius), Color(0.375,0,0,1)));

        IncreaseTime((int)(duration - maxIntensityTime) / 4);
        _effect->Render();
        ASSERT_TRUE(Assert_Color(_effect->GetColor(_lightInRadius), Color(0.25,0,0,1)));

        IncreaseTime((int)(duration - maxIntensityTime) / 4);
        _effect->Render();
        ASSERT_TRUE(Assert_Color(_effect->GetColor(_lightInRadius), Color(0.125,0,0,1)));

        //Just before the end
        IncreaseTime(((int)(duration - maxIntensityTime) / 4) - 1);
        _effect->Render();
        ASSERT_TRUE(Assert_Color(_effect->GetColor(_lightInRadius), Color(0.001,0,0,1)));

        //Effect disables itself when completed
        IncreaseTime(1);
        _effect->Render();
        ASSERT_FALSE(_effect->IsEnabled());

    }


}
