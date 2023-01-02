/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/
#ifndef LIBHUESTREAM_TEST_MOCKACTION_H_
#define LIBHUESTREAM_TEST_MOCKACTION_H_

#include "gmock/gmock.h"
#include "huestream/effect/lightscript/Action.h"

namespace huestream {

    class MockAction : public Action {
    public:
        MockAction() {}

        MockAction(std::string name, unsigned int layer, AnimationEffectPtr effect, long long int startTime) : Action(
                name, layer, effect, startTime) {}

        MOCK_CONST_METHOD0(GetTypeName, std::string());
    };

}// namespace huestream
#endif     // LIBHUESTREAM_TEST_MOCKACTION_H_
