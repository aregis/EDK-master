/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/
#ifndef LIBHUESTREAM_MOCKHUESTREAM_H
#define LIBHUESTREAM_MOCKHUESTREAM_H

#include "gmock/gmock.h"
#include "huestream/IHueStream.h"

namespace huestream {

    class MockHueStream : public huestream::IHueStream {
    public:
        MockHueStream() {}
        MOCK_METHOD0(GetConfig, ConfigPtr());
        MOCK_METHOD1(RegisterFeedbackHandler, void(FeedbackMessageHandlerPtr));
        MOCK_METHOD1(RegisterFeedbackCallback, void(FeedbackMessageCallback));
        MOCK_METHOD1(RegisterLightStateUpdatedHandler, void(LightStateChangedHandlerPtr));
        MOCK_METHOD0(ConnectBridge, void());
        MOCK_METHOD0(ConnectBridgeAsync, void());
        MOCK_METHOD0(ConnectBridgeBackground, void());
        MOCK_METHOD0(ConnectBridgeBackgroundAsync, void());
        MOCK_METHOD1(ConnectBridgeManualIp, void(const std::string &ipAddress));
        MOCK_METHOD1(ConnectBridgeManualIpAsync, void(const std::string &ipAddress));
        MOCK_METHOD1(ConnectManualBridgeInfo, void(BridgePtr bridge));
        MOCK_METHOD1(ConnectManualBridgeInfoAsync, void(BridgePtr bridge));
        MOCK_METHOD0(ConnectNewBridge, void());
        MOCK_METHOD0(ConnectNewBridgeAsync, void());
        MOCK_METHOD1(SelectGroup, void(GroupPtr group));
        MOCK_METHOD1(SelectGroupAsync, void(GroupPtr group));
        MOCK_METHOD1(SelectGroup, void(std::string ip));
        MOCK_METHOD1(SelectGroupAsync, void(std::string ip));
        MOCK_METHOD0(Start, void());
        MOCK_METHOD0(StartAsync, void());
        MOCK_METHOD0(ResetBridgeInfo, void());
        MOCK_METHOD0(ResetBridgeInfoAsync, void());
        MOCK_METHOD0(LoadBridgeInfo, void());
        MOCK_METHOD0(LoadBridgeInfoAsync, void());
        MOCK_METHOD0(AbortConnecting, void());
        MOCK_METHOD0(GetConnectionResult, ConnectResult());
        MOCK_METHOD0(IsStreamableBridgeLoaded, bool());
        MOCK_METHOD0(IsBridgeStreaming, bool());
        MOCK_METHOD0(GetLoadedBridge, BridgePtr());
        MOCK_METHOD0(GetLoadedBridgeStatus, BridgeStatus());
        MOCK_METHOD0(GetLoadedBridgeGroups, GroupListPtr());
        MOCK_METHOD0(RenderSingleFrame, void());
        MOCK_METHOD0(Stop, void());
        MOCK_METHOD0(LockMixer, void());
        MOCK_METHOD0(UnlockMixer, void());
        MOCK_METHOD1(AddEffect, void(EffectPtr newEffect));
        MOCK_METHOD1(AddLightScript, void(LightScriptPtr script));
        MOCK_METHOD1(GetEffectByName, EffectPtr(const std::string &name));
        MOCK_METHOD0(ShutDown, void());
        MOCK_METHOD0(PrepareRenderMixer, void());
        MOCK_METHOD0(GetStreamCounter, int32_t());
        MOCK_METHOD1(ChangeStreamingMode, void(StreamingMode mode));

    };

}// namespace spotlight
#endif     // LIBSPOTLIGHT_TEST_MOCKHUESTREAM_H_
