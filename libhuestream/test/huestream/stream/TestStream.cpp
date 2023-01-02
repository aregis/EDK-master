/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <memory>
#include <huestream/stream/Stream.h>
#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <huestream/config/Config.h>
#include <huestream/common/time/TimeManager.h>
#include <huestream/effect/Mixer.h>
#include <test/huestream/_mock/MockConnector.h>
#include "test/huestream/_mock/MockStreamStarter.h"
#include "test/huestream/_mock/MockStreamFactory.h"

using ::testing::AnyNumber;
using ::testing::_;
using ::testing::Expectation;
using ::testing::Invoke;
using ::testing::Return;

namespace huestream {

MATCHER_P(MatchStreamSendSequence, seq, "Incorrect sequence number") { return arg[11] == seq; };
MATCHER_P(MatchBridgeIpAddress, ipAddress, "Incorrect ip address") { return arg->GetIpAddress() == ipAddress; };


class TestStream : public testing::Test {
 public:
    std::shared_ptr<Stream> _stream;
    std::shared_ptr<MockStreamFactory> _mockStreamFactoryPtr;
    std::shared_ptr<MockStreamStarter> _mockStreamStarterPtr;
    std::shared_ptr<TimeManager> _timeManager;
    std::shared_ptr<MockConnector> _mockConnector;
    std::shared_ptr<Bridge> _bridge;

    virtual void SetUp() {
        _streamSettings = std::make_shared<StreamSettings>();
        _streamSettings->SetStreamingColorSpace(COLORSPACE_RGB);
        _streamSettings->SetUpdateFrequency(200);
        _appSettings = std::make_shared<AppSettings>();
        _appSettings->SetActivationOverride(ACTIVATION_OVERRIDELEVEL_SAMEGROUP);

        _mockConnector = std::make_shared<MockConnector>();
        _bridge = CreateBridge();

        _timeManager = std::make_shared<TimeManager>();
        _mixer = std::make_shared<Mixer>();

        _mockStreamFactoryPtr = std::make_shared<MockStreamFactory>();
        _mockStreamStarterPtr = std::make_shared<MockStreamStarter>();
        EXPECT_CALL(*_mockStreamFactoryPtr, CreateStreamStarter(_)).WillRepeatedly(
            DoAll(SaveArg<0>(&_mockStreamStarterPtr->bridge), Return(_mockStreamStarterPtr)));


        _stream = std::make_shared<Stream>(_streamSettings, _appSettings, _timeManager, _mockConnector,  _mockStreamFactoryPtr);
    }

    virtual void Teardown()
    {
        if (_stream != nullptr)
            _stream->Stop();
    }

    BridgePtr CreateBridge() {
        auto group1 = std::make_shared<Group>();
        group1->SetId("1");
        auto group2 = std::make_shared<Group>();
        group2->SetId("2");
        auto groupList = std::make_shared<GroupList>();
        groupList->push_back(group1);
        groupList->push_back(group2);
        auto bridge = std::make_shared<Bridge>(std::make_shared<BridgeSettings>());
        bridge->SetGroups(groupList);
        bridge->SetSelectedGroup("1");
        bridge->SetIpAddress("SOMEIP");
        bridge->SetUser("SOMEUSER");
        bridge->SetIsValidIp(true);
        bridge->SetIsAuthorized(true);
        bridge->SetId("SOMEID");
        bridge->SetModelId("BSB002");
        bridge->SetApiversion("1.24.0");
        bridge->SetSwversion("1940094000");
        bridge->SetClientKey("DD129216F1A50E5D1C0CB356325745F2");
        return bridge;
    }

    void start_correctly_without_renderthread() {
        Expectation start = EXPECT_CALL(*_mockStreamStarterPtr, StartStream(ACTIVATION_OVERRIDELEVEL_SAMEGROUP)).Times(1).WillOnce(
            Invoke(&*_mockStreamStarterPtr, &MockStreamStarter::ActivateSuccess));

        Expectation connect = EXPECT_CALL(*_mockConnector,
            Connect(MatchBridgeIpAddress("SOMEIP"), 2100)).Times(1).After(start).WillOnce(Return(true));

        EXPECT_CALL(*_mockConnector, Send(_, _)).Times(0);

        ASSERT_TRUE(_stream->Start(_bridge));
        ASSERT_TRUE(_stream->IsStreaming());
    }

    void stop_correctly() {
        EXPECT_CALL(*_mockConnector, Disconnect()).Times(1);
        EXPECT_CALL(*_mockStreamStarterPtr, Stop()).Times(1).WillOnce(
            Invoke(&*_mockStreamStarterPtr, &MockStreamStarter::DeactivateSuccess));
        _stream->Stop();
    }

    void expect_no_actions() {
        EXPECT_CALL(*_mockStreamStarterPtr, Start(_)).Times(0);
        EXPECT_CALL(*_mockStreamStarterPtr, Stop()).Times(0);
        EXPECT_CALL(*_mockConnector, Connect(_, _)).Times(0);
        EXPECT_CALL(*_mockConnector, Send(_, _)).Times(0);
        EXPECT_CALL(*_mockConnector, Disconnect()).Times(0);
    }

    virtual void TearDown() {
    }
    std::shared_ptr<Mixer> _mixer;
    std::shared_ptr<StreamSettings> _streamSettings;
    std::shared_ptr<AppSettings> _appSettings;
};

TEST_F(TestStream, StartWithRenderThread) {
    Expectation start = EXPECT_CALL(*_mockStreamStarterPtr, StartStream(ACTIVATION_OVERRIDELEVEL_SAMEGROUP)).Times(1).WillOnce(
        Invoke(&*_mockStreamStarterPtr, &MockStreamStarter::ActivateSuccess));
    Expectation connect = EXPECT_CALL(*_mockConnector,
                                      Connect(MatchBridgeIpAddress("SOMEIP"), 2100)).Times(1).After(
        start).WillOnce(Return(true));
    Expectation sendFirst = EXPECT_CALL(*_mockConnector, Send(MatchStreamSendSequence(0), _)).Times(1).After(
        start,
        connect).RetiresOnSaturation();
    Expectation sendSecond = EXPECT_CALL(*_mockConnector, Send(MatchStreamSendSequence(1), _)).Times(1).After(
        start,
        connect,
        sendFirst).RetiresOnSaturation();
    EXPECT_CALL(*_mockConnector, Send(_, _)).Times(AnyNumber()).After(start, connect, sendFirst, sendSecond);

    ASSERT_TRUE(_stream->StartWithRenderThread(_bridge));
    _timeManager->Sleep(100);

    ASSERT_TRUE(_stream->IsStreaming());

    stop_correctly();
}

TEST_F(TestStream, StartWithoutRenderThread) {
    start_correctly_without_renderthread();
    _timeManager->Sleep(100);

    EXPECT_CALL(*_mockConnector, Send(MatchStreamSendSequence(0), _)).Times(1);
    _stream->RenderSingleFrame();
    EXPECT_CALL(*_mockConnector, Send(MatchStreamSendSequence(1), _)).Times(1);
    _stream->RenderSingleFrame();

    stop_correctly();
}

TEST_F(TestStream, FailClientConnectRetries) {

    Expectation start = EXPECT_CALL(*_mockStreamStarterPtr, StartStream(ACTIVATION_OVERRIDELEVEL_SAMEGROUP)).Times(1).WillOnce(
        Invoke(&*_mockStreamStarterPtr, &MockStreamStarter::ActivateSuccess));
    Expectation connect = EXPECT_CALL(*_mockConnector, Connect(MatchBridgeIpAddress("SOMEIP"),
        2100)).Times(5).After(start).WillRepeatedly(Return(false));

    EXPECT_CALL(*_mockStreamStarterPtr, Stop()).Times(1).After(connect);
    EXPECT_CALL(*_mockConnector, Send(_, _)).Times(0);
    EXPECT_CALL(*_mockConnector, Disconnect()).Times(1);

    ASSERT_FALSE(_stream->Start(_bridge));
    ASSERT_FALSE(_stream->IsStreaming());
    _stream->RenderSingleFrame();

    expect_no_actions();
    _stream->Stop();
}

TEST_F(TestStream, FailCantActivate) {
    EXPECT_CALL(*_mockStreamStarterPtr, StartStream(ACTIVATION_OVERRIDELEVEL_SAMEGROUP)).Times(1).WillOnce(Return(false));
    EXPECT_CALL(*_mockConnector, Connect(_, _)).Times(0);
    EXPECT_CALL(*_mockConnector, Send(_, _)).Times(0);

    ASSERT_FALSE(_stream->Start(_bridge));
    ASSERT_FALSE(_stream->IsStreaming());
    _stream->RenderSingleFrame();

    expect_no_actions();
    _stream->Stop();
}

TEST_F(TestStream, FailInvalidBridge) {
    expect_no_actions();

    ASSERT_FALSE(_stream->Start(std::make_shared<Bridge>(std::make_shared<BridgeSettings>())));
    ASSERT_FALSE(_stream->IsStreaming());
    _stream->RenderSingleFrame();

    expect_no_actions();
    _stream->Stop();
}

TEST_F(TestStream, StartOnSameBridgeAndGroupAsRunningDoesNothing) {
    start_correctly_without_renderthread();

    expect_no_actions();
    ASSERT_TRUE(_stream->Start(_bridge));

    stop_correctly();
}

TEST_F(TestStream, StartWhenAlreadyRunningOnDifferentBridgeFirstStops) {
    start_correctly_without_renderthread();

    _bridge->SetId("SOMEOTHERID");
    _bridge->SetIpAddress("SOMEOTHERIP");

    Expectation disconnect = EXPECT_CALL(*_mockConnector, Disconnect()).Times(1);
    Expectation stop = EXPECT_CALL(*_mockStreamStarterPtr, Stop()).Times(1).WillOnce(
        Invoke(&*_mockStreamStarterPtr, &MockStreamStarter::DeactivateSuccess));
    Expectation start = EXPECT_CALL(*_mockStreamStarterPtr, StartStream(ACTIVATION_OVERRIDELEVEL_SAMEGROUP)).Times(1).After(disconnect, stop).WillOnce(
        Invoke(&*_mockStreamStarterPtr, &MockStreamStarter::ActivateSuccess));
    Expectation connect = EXPECT_CALL(*_mockConnector,
                                      Connect(MatchBridgeIpAddress("SOMEOTHERIP"), 2100)).Times(1).After(
        start).WillOnce(Return(true));

    ASSERT_TRUE(_stream->Start(_bridge));
    ASSERT_TRUE(_stream->IsStreaming());

    stop_correctly();
}

TEST_F(TestStream, StartWhenAlreadyRunningOnDifferentGroupFirstStops) {
    start_correctly_without_renderthread();

    _bridge->SetSelectedGroup("2");

    Expectation disconnect = EXPECT_CALL(*_mockConnector, Disconnect()).Times(1);
    Expectation stop = EXPECT_CALL(*_mockStreamStarterPtr, Stop()).Times(1);
    Expectation start = EXPECT_CALL(*_mockStreamStarterPtr, StartStream(ACTIVATION_OVERRIDELEVEL_SAMEGROUP)).Times(1).After(disconnect, stop).WillOnce(
        Invoke(&*_mockStreamStarterPtr, &MockStreamStarter::ActivateSuccess));
    Expectation connect = EXPECT_CALL(*_mockConnector,
                                      Connect(MatchBridgeIpAddress("SOMEIP"), 2100)).Times(1).After(
        start).WillOnce(Return(true));

    ASSERT_TRUE(_stream->Start(_bridge));
    ASSERT_TRUE(_stream->IsStreaming());

    stop_correctly();
}

TEST_F(TestStream, StopWhenNotRunningDoesNothing) {
    expect_no_actions();
    _stream->Stop();
}

TEST_F(TestStream, UpdateBridge) {
    start_correctly_without_renderthread();
    _timeManager->Sleep(100);

    EXPECT_CALL(*_mockConnector, Send(_, 16)).Times(1);
    _stream->RenderSingleFrame();
    EXPECT_CALL(*_mockConnector, Send(_, 25)).Times(1);
    auto bridgeCpy = CreateBridge();
    bridgeCpy->GetGroup()->AddLight("1", 0.1, 0.1, 0.1, "1", "LTC001");
    _stream->UpdateBridgeGroup(bridgeCpy);

    _stream->RenderSingleFrame();
    stop_correctly();
}

TEST_F(TestStream, UpdateBridgeRobustAgainstInvalidGroup) {
    start_correctly_without_renderthread();
    _timeManager->Sleep(100);

    EXPECT_CALL(*_mockConnector, Send(_, 16)).Times(1);
    _stream->RenderSingleFrame();
    EXPECT_CALL(*_mockConnector, Send(_, 16)).Times(1);
    auto bridgeCpy = CreateBridge();
    bridgeCpy->SetSelectedGroup("99");
    _stream->UpdateBridgeGroup(bridgeCpy);

    _stream->RenderSingleFrame();
    stop_correctly();
}

TEST_F(TestStream, UpdateBridgeRobustAgainstNotStarted) {
    _stream->UpdateBridgeGroup(_bridge);
}

TEST_F(TestStream, StopWithoutDeactivation) {
    start_correctly_without_renderthread();

    _bridge->GetGroup()->SetOwner("someoneelse");

    EXPECT_CALL(*_mockConnector, Disconnect()).Times(1);
    EXPECT_CALL(*_mockStreamStarterPtr, Stop()).Times(0);
    _stream->Stop(_bridge);
}

}
