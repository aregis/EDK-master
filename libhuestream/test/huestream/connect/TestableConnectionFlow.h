//
// Created by 310236882 on 6/28/2016.
//

#ifndef LIBHUESTREAM_TESTABLECONNECTIONFLOW_H
#define LIBHUESTREAM_TESTABLECONNECTIONFLOW_H

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <memory>
#include <functional>
#include <huestream/connect/ConnectionFlow.h>

using namespace testing;
using namespace huestream;



class TestableConnectionFlow : public ConnectionFlow {
public:

    explicit TestableConnectionFlow(const ConnectionFlowFactoryPtr &factory, StreamPtr stream, BridgeSettingsPtr bridgeSettings, AppSettingsPtr appSettings, BridgeStorageAccessorPtr storageAccessor)
            : ConnectionFlow(factory, stream, bridgeSettings, appSettings, storageAccessor) {
    }

    MOCK_METHOD1(NewMessage, void(const FeedbackMessage &m));
};

MATCHER_P(ActiveBridgeAttributes, bridge, "") {
    return ((arg->GetActiveBridge()->GetId() == bridge->GetId()) &&
            (arg->GetActiveBridge()->GetIpAddress() == bridge->GetIpAddress()) &&
            (arg->GetActiveBridge()->GetUser() == bridge->GetUser()) &&
            (arg->GetActiveBridge()->IsValidIp() == bridge->IsValidIp()) &&
            (arg->GetActiveBridge()->IsAuthorized() == bridge->IsAuthorized()));
};

MATCHER_P(BridgeIpAddress, ip, "") { return arg->GetIpAddress() == ip; };

MATCHER_P(BridgeId, id, "") { return arg->GetId() == id; };

MATCHER_P2(NumberOfBridgesAndActiveBridgeId, numberOfBridges, activeBridgeId, "") {
    return arg->GetActiveBridge()->GetId() == activeBridgeId &&
        arg->GetBridges()->size() == numberOfBridges;
};

MATCHER_P2(FbMessage, id, type, "") {
    return arg.GetId() == id && arg.GetMessageType() == type;
};

MATCHER_P3(FbMessageWithBridge, id, type, bridge, "") {
    return arg.GetId() == id &&
           arg.GetMessageType() == type &&
           arg.GetBridge()->GetId() == bridge->GetId() &&
           arg.GetBridge()->GetIpAddress() == bridge->GetIpAddress();
};

MATCHER_P4(FbMessageWithBridgeAndStatus, id, type, bridge, status, "") {
    return arg.GetId() == id &&
           arg.GetMessageType() == type &&
           arg.GetBridge()->GetId() == bridge->GetId() &&
           arg.GetBridge()->GetIpAddress() == bridge->GetIpAddress() &&
           arg.GetBridge()->GetStatus() == status;
};

MATCHER_P3(FbMessageWithBridgeList, id, type, size, "") {
    return arg.GetId() == id && arg.GetMessageType() == type && arg.GetBridgeList()->size() == size;
};

#endif //LIBHUESTREAM_TESTABLECONNECTIONFLOW_H
