/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/
#ifndef LIBHUESTREAM_TEST_MOCKEFFECT_H_
#define LIBHUESTREAM_TEST_MOCKEFFECT_H_

#include "gmock/gmock.h"
#include "huestream/common/data/Color.h"
#include "huestream/common/data/Light.h"

namespace huestream {

    class MockEffect : public Effect {
    public:
        MockEffect(std::string name) : Effect(name, 0) {};

        MockEffect(std::string name, unsigned int layer) : Effect(name, layer) {};

        MOCK_CONST_METHOD0(GetTypeName, std::string());

        MOCK_METHOD1(UpdateGroup, void(GroupPtr group));

        MOCK_METHOD0(Render, void());

        MOCK_METHOD1(GetColor, Color(LightPtr light));

        MOCK_METHOD0(IsFinished, bool());
    };

}// namespace huestream
#endif     // LIBHUESTREAM_TEST_MOCKEFFECT_H_
