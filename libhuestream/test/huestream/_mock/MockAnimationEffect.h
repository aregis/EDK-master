/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/
#ifndef LIBHUESTREAM_TEST_MOCKEFFECT_H_
#define LIBHUESTREAM_TEST_MOCKEFFECT_H_

#include <memory>
#include "gmock/gmock.h"
#include "huestream/common/data/Color.h"
#include "huestream/common/data/Light.h"
#include "huestream/effect/effects/base/AnimationEffect.h"

namespace huestream {

    class MockAnimationEffect : public AnimationEffect {
    public:
        MockAnimationEffect(std::string name) : MockAnimationEffect(name, 0) {}

        MockAnimationEffect(std::string name, unsigned int layer) : AnimationEffect(name, layer) {
            ON_CALL(*this, GetAnimations()).WillByDefault(testing::Return(std::make_shared<AnimationList>()));
        }

        MOCK_CONST_METHOD0(GetTypeName, std::string());

        MOCK_METHOD0(Render, void());

        MOCK_METHOD0(Enable, void());

        MOCK_METHOD1(GetColor, Color(LightPtr
                light));

        MOCK_METHOD0(IsFinished, bool());

        MOCK_METHOD0(RenderUpdate, void());

        MOCK_METHOD0(GetAnimations, AnimationListPtr());
        MOCK_METHOD1(SetSpeedAnimation, void(AnimationPtr
                speed));
        MOCK_METHOD1(SetPlayer, void(PlayerPtr
                player));

        MOCK_CONST_METHOD1(Serialize, void(JSONNode
                *node));

        MOCK_METHOD1(Deserialize, void(JSONNode
                *node));
    };

}// namespace huestream
#endif     // LIBHUESTREAM_TEST_MOCKEFFECT_H_
