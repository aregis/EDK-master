//
// Created by 310236882 on 6/28/2016.
//

#ifndef LIBHUESTREAM_MOCKBRIDGESEARCHER_H
#define LIBHUESTREAM_MOCKBRIDGESEARCHER_H

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <memory>
#include <functional>
#include <huestream/connect/ConnectionFlow.h>

using namespace testing;
using namespace huestream;



class MockBridgeSearcher : public IBridgeSearcher {

public:
    SearchCallbackHandler SearchNewCallback;

    MOCK_METHOD1(SearchNew, void(SearchCallbackHandler));

    MOCK_METHOD2(SearchNew, void(bool
            bruteForce, SearchCallbackHandler));

    MOCK_METHOD0(Abort, void());
};

#endif //LIBHUESTREAM_MOCKBRIDGESEARCHER_H
