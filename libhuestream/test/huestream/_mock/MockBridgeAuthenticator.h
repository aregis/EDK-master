//
// Created by 310236882 on 6/28/2016.
//

#ifndef LIBHUESTREAM_MOCKBRIDGEAUTHENTICATOR_H
#define LIBHUESTREAM_MOCKBRIDGEAUTHENTICATOR_H

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <memory>
#include <functional>
#include <huestream/connect/ConnectionFlow.h>

using namespace testing;
using namespace huestream;


class MockBridgeAuthenticator : public IBridgeAuthenticator {
public:
    AuthenticateCallbackHandler authenticate_callback;

    MOCK_METHOD3(Authenticate, void(
            BridgePtr i, AppSettingsPtr appSetting, AuthenticateCallbackHandler
            cb));

    MOCK_METHOD0(Abort, void());
};

#endif //LIBHUESTREAM_MOCKBRIDGEAUTHENTICATOR_H
