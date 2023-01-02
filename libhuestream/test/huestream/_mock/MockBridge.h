/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/
#ifndef LIBHUESTREAM_TEST_MOCKBRIDGE_H_
#define LIBHUESTREAM_TEST_MOCKBRIDGE_H_

#include "gmock/gmock.h"
#include "huestream/common/data/Bridge.h"

namespace huestream {

    class MockBridge : public Bridge {
    public:
        explicit MockBridge(BridgeSettingsPtr bridgeSettings): Bridge(bridgeSettings) {}
        MOCK_CONST_METHOD0(IsReadyToStream, bool());
        MOCK_CONST_METHOD0(IsStreaming, bool());
        MOCK_CONST_METHOD0(GetStatus, BridgeStatus());
        MOCK_CONST_METHOD0(GetGroup, GroupPtr());
    };

}// namespace huestream
#endif     // LIBHUESTREAM_TEST_MOCKBRIDGE_H_
