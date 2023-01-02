/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include "gtest/gtest.h"
#include <memory>

#include "huestream/effect/Mixer.h"
#include "test/huestream/_mock/MockEffect.h"
#include "test/huestream/_mock/MockTimeline.h"

using ::testing::Return;
using ::testing::_;
using ::testing::Sequence;
using ::testing::Property;
using ::testing::AllOf;
using ::testing::Pointee;
using ::testing::Values;
using namespace huestream;

class TestMixer : public testing::TestWithParam<bool> {
protected:
    std::shared_ptr<Mixer> _mixer;
    std::shared_ptr<Group> _group;
    std::shared_ptr<MockTimeline> _mockTimeline;
    std::vector<std::shared_ptr<MockEffect>> _effects;
    std::vector<Sequence> _sequences;

    const static int EffectLayer0 = 0;
    const static int EffectLayer1 = 1;
    const static int EffectLayer2 = 2;

    virtual void SetUp() {
        _group = std::make_shared<Group>();
        _group->AddLight("1", 0.1, 0.1);
        _group->AddLight("3", 0.5, 0.6);
        _mockTimeline = std::make_shared<MockTimeline>();
        _mixer = std::make_shared<Mixer>();
        _mixer->SetGroup(_group);
    }

    virtual void TearDown() {
        for (auto effect : _effects) {
            effect.reset();
        }
        _effects.clear();
        _mixer.reset();
        _group.reset();
        _sequences.clear();
        _mockTimeline.reset();
    }

    virtual void AddEffectWithLayer(unsigned int layer) {
        auto effect = std::make_shared<MockEffect>("testEffect");
        effect->SetLayer(layer);
        EXPECT_CALL(*effect, UpdateGroup(_group));
        _mixer->AddEffect(effect);
        effect->Enable();
        EXPECT_CALL(*effect, Render());
        EXPECT_CALL(*effect, IsFinished()).WillRepeatedly(Return(false));
        _sequences.push_back(Sequence());
        _effects.push_back(effect);
    }

    virtual void AddEffectWithLayerNotRendered(unsigned int layer) {
        auto effect = std::make_shared<MockEffect>("testEffect");
        effect->SetLayer(layer);
        _sequences.push_back(Sequence());
        _effects.push_back(effect);
        EXPECT_CALL(*effect, UpdateGroup(_group));
        _mixer->AddEffect(effect);
    }

    virtual void SetExpectEffectGet(int effectIndex, Location pos, Color color) {
        auto effect = _effects[effectIndex];
        EXPECT_CALL(*effect, GetColor(Pointee(Property(&Light::GetPosition, AllOf(Property(&Location::GetX, pos.GetX()),
                                                                                  Property(&Location::GetY,
                                                                                           pos.GetY()))))))
                .InSequence(_sequences[effectIndex])
                .WillOnce(Return(color));
    }

    virtual void SetExpectionForEffectIsFinished(int effectIndex, bool isFinished) {
        auto effect = _effects[effectIndex];
        EXPECT_CALL(*effect, IsFinished()).WillRepeatedly(Return(isFinished));
    }

    virtual void SetExpectEffectNoGet(int effectIndex) {
        auto effect = _effects[effectIndex];
        EXPECT_CALL(*effect, GetColor(_)).Times(0);
    }

    virtual void AssertColors(Colors colors) {
        auto lights = _group->GetLights();
        ASSERT_EQ(colors.size(), lights->size());

        auto i = 0;
        for (auto light : *lights) {
            Color s = light->GetColor();
            AssertColor(colors[i++], s);
        }
    }

    virtual void AssertColor(const Color &expectedColor, const Color &actualColor) {
        EXPECT_EQ(expectedColor.GetR(), actualColor.GetR());
        EXPECT_EQ(expectedColor.GetG(), actualColor.GetG());
        EXPECT_EQ(expectedColor.GetB(), actualColor.GetB());
    }
};

TEST_F(TestMixer, AnEffectIsAppliedToLights) {
    auto color1 = Color(1.0, 0.0, 0.0);
    auto color2 = Color(0.0, 1.0, 0.0);
    auto foregroundLayer = EffectLayer0;

    AddEffectWithLayer(0);
    SetExpectEffectGet(foregroundLayer, Location(0.1, 0.1), color1);
    SetExpectEffectGet(foregroundLayer, Location(0.5, 0.6), color2);
    _mixer->Render();

    AssertColors(Colors::Create(2, color1, color2));
}

TEST_F(TestMixer, ColorMixing_SolidBackgroundAndForegroundColors) {
    auto color1 = Color(123, 0.456, 0.654);
    auto color2 = Color(122, 0.457, 0.653);
    auto backgroundLayer = EffectLayer0;
    auto foregroundLayer = EffectLayer1;

    AddEffectWithLayer(0);
    AddEffectWithLayer(1);

    SetExpectEffectGet(backgroundLayer, Location(0.1, 0.1), Color());
    SetExpectEffectGet(backgroundLayer, Location(0.5, 0.6), Color());

    SetExpectEffectGet(foregroundLayer, Location(0.1, 0.1), color1);
    SetExpectEffectGet(foregroundLayer, Location(0.5, 0.6), color2);

    _mixer->Render();
    AssertColors(Colors::Create(2, color1, color2));
}

TEST_F(TestMixer, ColorMixing_TransparentColorForegroundSolidBackground) {
    auto color1 = Color(123, 0.456, 0.654);
    auto color2 = Color(122, 0.457, 0.653);
    auto transparentColor = Color(1, 0.1, 0.2, 0);
    auto backgroundLayer = EffectLayer0;
    auto foregroundLayer = EffectLayer1;

    AddEffectWithLayer(0);
    AddEffectWithLayer(1);

    SetExpectEffectGet(backgroundLayer, Location(0.1, 0.1), color1);
    SetExpectEffectGet(backgroundLayer, Location(0.5, 0.6), color2);

    SetExpectEffectGet(foregroundLayer, Location(0.1, 0.1), transparentColor);
    SetExpectEffectGet(foregroundLayer, Location(0.5, 0.6), transparentColor);


    _mixer->Render();
    AssertColors(Colors::Create(2, color1, color2));
}

TEST_F(TestMixer, ColorMixing_SemiTransparentColorForegroundSolidBackground) {
    auto color1 = Color(0.2, 0.2, 0.2);
    auto color2 = Color(0.0, 0.0, 0.0);
    auto semiTransparentColor1 = Color(1, 1, 1, 0.5);
    auto semiTransparentColor2 = Color(1, 1, 1, 0.2);
    auto resultColor1 = Color(0.6, 0.6, 0.6);
    auto resultColor2 = Color(0.2, 0.2, 0.2);
    auto backgroundLayer = EffectLayer0;
    auto foregroundLayer = EffectLayer1;

    AddEffectWithLayer(0);
    AddEffectWithLayer(1);

    SetExpectEffectGet(backgroundLayer, Location(0.1, 0.1), color1);
    SetExpectEffectGet(backgroundLayer, Location(0.5, 0.6), color2);

    SetExpectEffectGet(foregroundLayer, Location(0.1, 0.1), semiTransparentColor1);
    SetExpectEffectGet(foregroundLayer, Location(0.5, 0.6), semiTransparentColor2);


    _mixer->Render();
    AssertColors(Colors::Create(2, resultColor1, resultColor2));
}

TEST_F(TestMixer, ColorMixing_SemiTransparentColorForegroundSemiTransparentColorMiddleSolidBackground) {
    auto solidColorBackground = Color(0.2, 0.2, 0.2, 1);
    auto semiTransparentColorMiddle = Color(1, 1, 1, 0.5);
    auto semiTransparentColorForeground = Color(1, 1, 1, 0.5);
    auto resultColor = Color(0.8, 0.8, 0.8);

    auto backgroundLayer = EffectLayer0;
    auto middleLayer = EffectLayer1;
    auto foregroundLayer = EffectLayer2;

    AddEffectWithLayer(0);
    AddEffectWithLayer(1);
    AddEffectWithLayer(2);

    SetExpectEffectGet(backgroundLayer, Location(0.1, 0.1), solidColorBackground);
    SetExpectEffectGet(middleLayer, Location(0.1, 0.1), semiTransparentColorMiddle);
    SetExpectEffectGet(foregroundLayer, Location(0.1, 0.1), semiTransparentColorForeground);

    SetExpectEffectGet(backgroundLayer, Location(0.5, 0.6), solidColorBackground);
    SetExpectEffectGet(middleLayer, Location(0.5, 0.6), semiTransparentColorMiddle);
    SetExpectEffectGet(foregroundLayer, Location(0.5, 0.6), semiTransparentColorForeground);

    _mixer->Render();
    AssertColors(Colors::Create(2, resultColor, resultColor));
}

TEST_F(TestMixer, EffectIsNotCalledWhenNotEnabled) {
    auto foregroundLayer = EffectLayer0;

    AddEffectWithLayerNotRendered(0);
    SetExpectEffectNoGet(foregroundLayer);
    _mixer->Render();
    AssertColors(Colors::Create(2, Color(0.0, 0.0, 0.0), Color(0.0, 0.0, 0.0)));
}


TEST_F(TestMixer, RemoveEffectWhenStateOfEffectIsFinished) {
    auto foregroundLayer = EffectLayer0;

    AddEffectWithLayerNotRendered(0);
    SetExpectEffectNoGet(foregroundLayer);
    _effects[0]->Finish();
    _mixer->Render();

    AssertColors(Colors::Create(2, Color(0.0, 0.0, 0.0), Color(0.0, 0.0, 0.0)));
}

TEST_F(TestMixer, GroupChangeIsUpdatedToEffects) {
    auto effect = std::make_shared<MockEffect>("testEffect");
    effect->SetLayer(0);

    EXPECT_CALL(*effect, UpdateGroup(_group));
    _mixer->AddEffect(effect);

    auto group = std::make_shared<Group>();

    EXPECT_CALL(*effect, UpdateGroup(group));
    _mixer->SetGroup(group);
}

TEST_F(TestMixer, IsRobustAgainstInvalidGroup) {
    auto effect = std::make_shared<MockEffect>("testEffect");
    effect->SetLayer(0);

    EXPECT_CALL(*effect, UpdateGroup(_group));
    _mixer->AddEffect(effect);

    EXPECT_CALL(*effect, UpdateGroup(_)).Times(0);
    _mixer->SetGroup(nullptr);
}

INSTANTIATE_TEST_CASE_P(RemoveOrRetainColorAfterEffect, TestMixer, Values(true, false));

TEST_P(TestMixer, RemoveOrRetainColorAfterEffect) {
    auto appSettings = std::make_shared<AppSettings>();
    appSettings->SetLightsRetainColor(GetParam());
    _mixer = std::make_shared<Mixer>(appSettings);
    _mixer->SetGroup(_group);

    auto color1 = Color(1.0, 0.0, 0.0);
    auto color2 = Color(0.0, 1.0, 0.0);
    auto foregroundLayer = EffectLayer0;

    AddEffectWithLayer(0);
    SetExpectEffectGet(foregroundLayer, Location(0.1, 0.1), color1);
    SetExpectEffectGet(foregroundLayer, Location(0.5, 0.6), color2);
    SetExpectionForEffectIsFinished(foregroundLayer, false);
    _mixer->Render();

    AssertColors(Colors::Create(2, color1, color2));

    _effects[0]->Finish();
    _mixer->Render();

    if (GetParam()) {
        AssertColors(Colors::Create(2, color1, color2));
    } else {
        AssertColors(Colors::Create(2, Color(0.0, 0.0, 0.0), Color(0.0, 0.0, 0.0)));
    }
}
