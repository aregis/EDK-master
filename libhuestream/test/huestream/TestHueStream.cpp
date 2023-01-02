/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <huestream/config/ObjectBuilder.h>
#include <huestream/HueStream.h>
#include <test/huestream/_mock/MockBridge.h>
#include <test/huestream/_mock/MockStream.h>
#include <test/huestream/_mock/MockConnect.h>
#include <test/huestream/_mock/MockConnectionMonitor.h>
#include <test/huestream/_stub/StubMessageDispatcher.h>
#include <huestream/common/time/TimeManager.h>
#include <huestream/common/time/TimeProviderProvider.h>
#include <test/huestream/_mock/MockTimeManager.h>
#include <test/huestream/_mock/MockBridgeHttpClient.h>
#include <test/huestream/_mock/MockConnector.h>
#include <test/huestream/_mock/MockMixer.h>
#include "gtest/gtest.h"

#include <support/util/Factory.h>
#include <test/huestream/_mock/MockBridgeStorageAccessor.h>
#include <test/huestream/_mock/MockMessageTranslator.h>
#include <test/huestream/_mock/MockBasicGroupLightController.h>

using namespace huestream;
using support::Factory;

using ::testing::Return;
using ::testing::_;
using ::testing::Sequence;
using ::testing::Property;
using ::testing::AllOf;
using ::testing::Pointee;
using ::testing::SaveArg;

class LightStateChangedHandler: public ILightStateChangedHandler {
 public:
    LightListPtr _lights;
    void OnLightStateUpdated(LightListPtr lights) override {
        _lights = std::move(lights);
    }
};

class TestHuestream : public testing::Test {
 protected:
    virtual void SetUp() {
        Serializable::SetObjectBuilder(std::make_shared<ObjectBuilder>(nullptr));
        _handler = std::make_shared<LightStateChangedHandler>();

        auto config = std::make_shared<Config>("test", "pc", PersistenceEncryptionKey{"encryption_key"});

        _mockHttpClient = std::make_shared<MockBridgeHttpClient>();
        Factory<std::unique_ptr<IBridgeHttpClient>()>::set_factory_method([this](){
            return support::make_unique<MockWrapperBridgeHttpClient>(_mockHttpClient);});

        _mockTimeManager = std::make_shared<MockTimeManager>();
        Factory<std::unique_ptr<ITimeManager>()>::set_factory_method(
                [this](){ return support::make_unique<MockWrapperTimeManager>(_mockTimeManager); });

        _mockConnectionMonitor = std::make_shared<MockConnectionMonitor>();
        EXPECT_CALL(*_mockConnectionMonitor, SetFeedbackMessageCallback(_)).Times(1).WillOnce(SaveArg<0>(&_mockConnectionMonitor->callback));

        Factory<std::unique_ptr<IConnectionMonitor>(BridgeHttpClientPtr, AppSettingsPtr)>::set_factory_method(
                [this](){ return support::make_unique<MockWrapperConnectionMonitor>(_mockConnectionMonitor); });

        _mockMixer = std::make_shared<MockMixer>();
        Factory<std::unique_ptr<IMixer>(AppSettingsPtr)>::set_factory_method([this](){ return support::make_unique<MockWrapperMixer>(_mockMixer); });

        _mockBridgeStorageAccesser = std::make_shared<MockBridgeStorageAccessor>();
        Factory<std::unique_ptr<IBridgeStorageAccessor>(const std::string &,BridgeSettingsPtr)>::set_factory_method(
                [this](){ return support::make_unique<MockWrapperBridgeStorageAccessor>(_mockBridgeStorageAccesser); });

        Factory<std::unique_ptr<IMessageDispatcher>()>::set_factory_method(
                [this](){ return support::make_unique<StubMessageDispatcher>(); });

        _mockMessageTranslator = std::make_shared<MockMessageTranslator>();
        Factory<std::unique_ptr<IMessageTranslator>(std::string)>::set_factory_method(
                [this](){ return support::make_unique<MockWrapperMessageTranslator>(_mockMessageTranslator); });

        _mockConnector = std::make_shared<MockConnector>();
        Factory<std::unique_ptr<IConnector>(ConfigPtr)>::set_factory_method(
                [this](){ return support::make_unique<MockWrapperConnector>(_mockConnector); });

        _mockStream = std::make_shared<MockStream>();
        Factory<
            std::unique_ptr<IStream>(StreamSettingsPtr,
                    AppSettingsPtr,
                    TimeManagerPtr,
                    ConnectorPtr)>::set_factory_method(
                [this](){ return support::make_unique<MockWrapperStream>(_mockStream); });

        _mockConnect = std::make_shared<MockConnect>();
        Factory<std::unique_ptr<IConnect>(BridgeHttpClientPtr,
                         MessageDispatcherPtr,
                         BridgeSettingsPtr,
                         AppSettingsPtr,
                         StreamPtr,
                         BridgeStorageAccessorPtr)>::set_factory_method(
                [this](){ return support::make_unique<MockWrapperConnect>(_mockConnect); });
        EXPECT_CALL(*_mockConnect, SetFeedbackMessageCallback(_)).Times(1).WillOnce(SaveArg<0>(&_mockConnect->callback));

        _mockGroupController = std::make_shared<MockBasicGroupLightController>();
        Factory<std::unique_ptr<IBasicGroupLightController>(BridgeHttpClientPtr)>::set_factory_method(
                [this]() { return support::make_unique<MockWrapperBasicGroupLightController>(_mockGroupController); });

        huestream = std::make_shared<HueStream>(config);
    }

    virtual void TearDown() {
        auto bridge = std::make_shared<MockBridge>(std::make_shared<BridgeSettings>());
        EXPECT_CALL(*bridge, IsStreaming()).Times(1).WillOnce(Return(true));
        huestream->SetActiveBridge(bridge);

        {
            InSequence dummy;

            EXPECT_CALL(*_mockConnect, Abort());
            EXPECT_CALL(*_mockConnect, WaitUntilReady());
            EXPECT_CALL(*_mockConnect, StopStream(_));
            EXPECT_CALL(*_mockConnect, WaitUntilReady());
            EXPECT_CALL(*_mockConnectionMonitor, Stop());
        }

        huestream->ShutDown();
        Serializable::SetObjectBuilder(std::make_shared<ObjectBuilder>(nullptr));
    }

 protected:
    std::shared_ptr<HueStream> huestream;
    std::shared_ptr<MockStream> _mockStream;
    std::shared_ptr<LightStateChangedHandler> _handler;
    std::shared_ptr<MockConnect> _mockConnect;
    std::shared_ptr<MockConnectionMonitor> _mockConnectionMonitor;
    std::shared_ptr<MockConnector> _mockConnector;
    std::shared_ptr<MockBridgeHttpClient> _mockHttpClient;
    std::shared_ptr<MockTimeManager> _mockTimeManager;
    std::shared_ptr<MockMixer> _mockMixer;
    std::shared_ptr<MockBridgeStorageAccessor> _mockBridgeStorageAccesser;
    std::shared_ptr<MockMessageTranslator> _mockMessageTranslator;
    std::shared_ptr<MockBasicGroupLightController> _mockGroupController;

    BridgePtr CreateNotStreamingBridge() const {
        auto bridge = std::make_shared<Bridge>(std::make_shared<BridgeSettings>());
        auto group = std::make_shared<Group>();
        group->SetId("2");
        bridge->GetGroups()->push_back(group);
        bridge->SelectGroup("2");
        bridge->SetIpAddress("SOMEIP");
        bridge->SetUser("SOMEUSER");
        bridge->SetIsValidIp(true);
        bridge->SetIsAuthorized(true);
        bridge->SetId("SOMEID");
        bridge->SetModelId("BSB002");
        bridge->SetApiversion("1.24.0");
        bridge->SetSwversion("1940094000");
        bridge->SetClientKey("DD129216F1A50E5D1C0CB356325745F2");
        EXPECT_FALSE(bridge->IsStreaming());
        return bridge;
    }

    BridgePtr CreateStreamingBridge() const {
        auto bridge = CreateNotStreamingBridge();
        bridge->GetGroup()->SetActive(true);
        bridge->GetGroup()->SetOwner("SOMEUSER");
        EXPECT_TRUE(bridge->IsStreaming());
        return bridge;
    }

    void expect_setup_stream(BridgePtr bridge) {
        EXPECT_CALL(*_mockMixer, Lock());
        EXPECT_CALL(*_mockMixer, SetGroup(bridge->GetGroup()));
        EXPECT_CALL(*_mockStream, UpdateBridgeGroup(bridge));
        EXPECT_CALL(*_mockMixer, Unlock());
    }

    void expect_no_setup_stream() {
        EXPECT_CALL(*_mockMixer, Lock()).Times(0);
        EXPECT_CALL(*_mockMixer, SetGroup(_)).Times(0);
        EXPECT_CALL(*_mockStream, UpdateBridgeGroup(_)).Times(0);
        EXPECT_CALL(*_mockMixer, Unlock()).Times(0);
    }
};

TEST_F(TestHuestream, Create) {

}

TEST_F(TestHuestream, GetConfig) {
    ASSERT_TRUE(huestream->GetConfig() != nullptr);
}

TEST_F(TestHuestream, LoadBridgeInfo) {
    EXPECT_CALL(*_mockConnect, Load());
    EXPECT_CALL(*_mockConnect, WaitUntilReady());
    huestream->LoadBridgeInfo();
}

TEST_F(TestHuestream, LoadBridgeInfoAsync) {
    EXPECT_CALL(*_mockConnect, Load());
    huestream->LoadBridgeInfoAsync();
}

TEST_F(TestHuestream, ConnectManualBridgeInfo) {
    auto bridge = std::make_shared<Bridge>(std::make_shared<BridgeSettings>());
    EXPECT_CALL(*_mockConnect, SetManual(bridge));
    EXPECT_CALL(*_mockConnect, WaitUntilReady());
    huestream->ConnectManualBridgeInfo(bridge);
}

TEST_F(TestHuestream, ConnectManualBridgeInfoAsync) {
    auto bridge = std::make_shared<Bridge>(std::make_shared<BridgeSettings>());
    EXPECT_CALL(*_mockConnect, SetManual(bridge));
    huestream->ConnectManualBridgeInfoAsync(bridge);
}

TEST_F(TestHuestream, ResetBridgeInfo) {
    EXPECT_CALL(*_mockConnect, ResetBridge());
    EXPECT_CALL(*_mockConnect, WaitUntilReady());
    huestream->ResetBridgeInfo();
}

TEST_F(TestHuestream, ResetBridgeInfoAsync) {
    EXPECT_CALL(*_mockConnect, ResetBridge());
    huestream->ResetBridgeInfoAsync();
}

TEST_F(TestHuestream, ResetAll) {
    EXPECT_CALL(*_mockConnect, ResetAllData());
    EXPECT_CALL(*_mockConnect, WaitUntilReady());
    huestream->ResetAllPersistentData();
}

TEST_F(TestHuestream, ResetAllAsync) {
    EXPECT_CALL(*_mockConnect, ResetAllData());
    huestream->ResetAllPersistentDataAsync();
}

TEST_F(TestHuestream, ConnectBridge) {
    EXPECT_CALL(*_mockConnect, ConnectToBridge());
    EXPECT_CALL(*_mockConnect, WaitUntilReady());
    huestream->ConnectBridge();
}

TEST_F(TestHuestream, ConnectBridgeAsync) {
    EXPECT_CALL(*_mockConnect, ConnectToBridge());
    huestream->ConnectBridgeAsync();
}

TEST_F(TestHuestream, ConnectNewBridge) {
    EXPECT_CALL(*_mockConnect, ConnectToNewBridge());
    EXPECT_CALL(*_mockConnect, WaitUntilReady());
    huestream->ConnectNewBridge();
}

TEST_F(TestHuestream, ConnectNewBridgeAsync) {
    EXPECT_CALL(*_mockConnect, ConnectToNewBridge());
    huestream->ConnectNewBridgeAsync();
}

TEST_F(TestHuestream, ConnectToBridgeBackground) {
    EXPECT_CALL(*_mockConnect, ConnectToBridgeBackground());
    EXPECT_CALL(*_mockConnect, WaitUntilReady());
    huestream->ConnectBridgeBackground();
}

TEST_F(TestHuestream, ConnectToBridgeBackgroundAsync) {
    EXPECT_CALL(*_mockConnect, ConnectToBridgeBackground());
    huestream->ConnectBridgeBackgroundAsync();
}

TEST_F(TestHuestream, SelectGroupStr) {
    auto group = "1";
    EXPECT_CALL(*_mockConnect, SelectGroup(group));
    EXPECT_CALL(*_mockConnect, WaitUntilReady());
    huestream->SelectGroup(group);
}

TEST_F(TestHuestream, SelectGroupPtr) {
    auto group = std::make_shared<Group>();
    group->SetId("1");
    EXPECT_CALL(*_mockConnect, SelectGroup("1"));
    EXPECT_CALL(*_mockConnect, WaitUntilReady());
    huestream->SelectGroup(group);
}

TEST_F(TestHuestream, SelectGroupStrAsync) {
    auto group = "1";
    EXPECT_CALL(*_mockConnect, SelectGroup(group));
    huestream->SelectGroupAsync(group);
}

TEST_F(TestHuestream, SelectGroupPtrAsync) {
    auto group = std::make_shared<Group>();
    group->SetId("1");
    EXPECT_CALL(*_mockConnect, SelectGroup("1"));
    huestream->SelectGroupAsync(group);
}

TEST_F(TestHuestream, Start) {
    auto bridge = std::make_shared<MockBridge>(std::make_shared<BridgeSettings>());
    huestream->SetActiveBridge(bridge);

    EXPECT_CALL(*_mockConnect, StartStream(_));
    EXPECT_CALL(*_mockConnect, WaitUntilReady());

    huestream->Start();
}

TEST_F(TestHuestream, StartAsync) {
    auto bridge = std::make_shared<MockBridge>(std::make_shared<BridgeSettings>());
    huestream->SetActiveBridge(bridge);

    EXPECT_CALL(*_mockConnect, StartStream(_));

    huestream->StartAsync();
}

TEST_F(TestHuestream, Stop) {
    auto bridge = std::make_shared<MockBridge>(std::make_shared<BridgeSettings>());
    huestream->SetActiveBridge(bridge);
    EXPECT_CALL(*bridge, IsStreaming()).WillRepeatedly(Return(true));
    EXPECT_CALL(*_mockConnect, StopStream(_)).Times(1);
    EXPECT_CALL(*_mockConnect, WaitUntilReady()).Times(1);
    huestream->Stop();
}

TEST_F(TestHuestream, StopAsync) {
    auto bridge = std::make_shared<MockBridge>(std::make_shared<BridgeSettings>());
    huestream->SetActiveBridge(bridge);
    EXPECT_CALL(*bridge, IsStreaming()).WillRepeatedly(Return(true));
    EXPECT_CALL(*_mockConnect, StopStream(_)).Times(1);
    huestream->StopAsync();
}

TEST_F(TestHuestream, StopIgnored) {
    auto bridge = std::make_shared<MockBridge>(std::make_shared<BridgeSettings>());
    huestream->SetActiveBridge(bridge);
    EXPECT_CALL(*bridge, IsStreaming()).WillRepeatedly(Return(false));
    EXPECT_CALL(*_mockConnect, StopStream(_)).Times(0);
    EXPECT_CALL(*_mockConnect, WaitUntilReady()).Times(0);
    huestream->Stop();
}

TEST_F(TestHuestream, StopAsyncIgnored) {
    auto bridge = std::make_shared<MockBridge>(std::make_shared<BridgeSettings>());
    huestream->SetActiveBridge(bridge);
    EXPECT_CALL(*bridge, IsStreaming()).WillRepeatedly(Return(false));
    EXPECT_CALL(*_mockConnect, StopStream(_)).Times(0);
    huestream->StopAsync();
}

TEST_F(TestHuestream, AbortConnecting) {
    EXPECT_CALL(*_mockConnect, Abort()).Times(1);
    EXPECT_CALL(*_mockConnect, WaitUntilReady());
    huestream->AbortConnecting();
}

TEST_F(TestHuestream, AbortConnectingAsync) {
    EXPECT_CALL(*_mockConnect, Abort()).Times(1);
    huestream->AbortConnecting();
}

TEST_F(TestHuestream, GetConnectionResult) {
    EXPECT_CALL(*_mockConnect, GetResult()).WillOnce(Return(Uninitialized));
    huestream->GetConnectionResult();
}

TEST_F(TestHuestream, IsStreamableBridgeLoaded) {
    auto bridge = std::make_shared<MockBridge>(std::make_shared<BridgeSettings>());
    huestream->SetActiveBridge(bridge);
    EXPECT_CALL(*bridge, IsReadyToStream()).WillRepeatedly(Return(true));
    ASSERT_TRUE(huestream->IsStreamableBridgeLoaded());
    EXPECT_CALL(*bridge, IsReadyToStream()).WillRepeatedly(Return(false));
    ASSERT_FALSE(huestream->IsStreamableBridgeLoaded());
}

TEST_F(TestHuestream, IsBridgeStreaming) {
    auto bridge = std::make_shared<MockBridge>(std::make_shared<BridgeSettings>());
    auto group = std::make_shared<Group>();
    group->SetId("2");
    bridge->GetGroups()->push_back(group);
    huestream->SetActiveBridge(bridge);
    EXPECT_CALL(*bridge, IsStreaming()).WillRepeatedly(Return(true));
    ASSERT_TRUE(huestream->IsBridgeStreaming());
    EXPECT_CALL(*bridge, IsStreaming()).WillRepeatedly(Return(false));
    ASSERT_FALSE(huestream->IsBridgeStreaming());
}

TEST_F(TestHuestream, GetLoadedBridgeStatus) {
    auto bridge = std::make_shared<MockBridge>(std::make_shared<BridgeSettings>());
    auto group = std::make_shared<Group>();
    group->SetId("2");
    bridge->GetGroups()->push_back(group);
    huestream->SetActiveBridge(bridge);
    EXPECT_CALL(*bridge, GetStatus()).WillRepeatedly(Return(BRIDGE_READY));
    ASSERT_EQ(BRIDGE_READY, huestream->GetLoadedBridgeStatus());
    EXPECT_CALL(*bridge, GetStatus()).WillRepeatedly(Return(BRIDGE_INVALID_CLIENTKEY));
    ASSERT_EQ(BRIDGE_INVALID_CLIENTKEY, huestream->GetLoadedBridgeStatus());
}

TEST_F(TestHuestream, NewFeedbackMessage_PROCEDURE_FINISHED_updates_bridge_group_if_streaming) {
    auto bridge = CreateStreamingBridge();
    auto message = FeedbackMessage(FeedbackMessage::REQUEST_TYPE_CONNECT, FeedbackMessage::ID_USERPROCEDURE_FINISHED, bridge);

    expect_setup_stream(bridge);
    _mockConnect->callback(message);
}

TEST_F(TestHuestream, NewFeedbackMessage_PROCEDURE_FINISHED_doesnt_update_bridge_group_if_not_streaming) {
    auto bridge = CreateNotStreamingBridge();
    auto message = FeedbackMessage(FeedbackMessage::REQUEST_TYPE_CONNECT, FeedbackMessage::ID_USERPROCEDURE_FINISHED, bridge);

    expect_no_setup_stream();
    _mockConnect->callback(message);
}

TEST_F(TestHuestream, NewFeedbackMessage_TYPE_INTERNAL_updates_bridge_group_if_streaming) {
    auto bridge = CreateStreamingBridge();
    auto message = FeedbackMessage(FeedbackMessage::REQUEST_TYPE_INTERNAL, FeedbackMessage::ID_LIGHTS_UPDATED, bridge);

    expect_setup_stream(bridge);
    _mockConnect->callback(message);
}

TEST_F(TestHuestream, NewFeedbackMessage_TYPE_INTERNAL_doesnt_update_bridge_group_if_not_streaming) {
    auto bridge = CreateNotStreamingBridge();
    auto message = FeedbackMessage(FeedbackMessage::REQUEST_TYPE_INTERNAL, FeedbackMessage::ID_LIGHTS_UPDATED, bridge);

    expect_no_setup_stream();
    _mockConnect->callback(message);
}

TEST_F(TestHuestream, NewFeedbackMessage_TYPE_INTERNAL_starts_monitor_if_connectable) {
    auto bridge = CreateNotStreamingBridge();
    auto message = FeedbackMessage(FeedbackMessage::REQUEST_TYPE_INTERNAL, FeedbackMessage::ID_STREAMING_DISCONNECTED, bridge);
    EXPECT_CALL(*_mockConnectionMonitor, Start(bridge, 15000));
    _mockConnect->callback(message);
}

TEST_F(TestHuestream, NewFeedbackMessage_TYPE_INTERNAL_starts_monitor_if_streaming) {
    auto bridge = CreateStreamingBridge();
    auto message = FeedbackMessage(FeedbackMessage::REQUEST_TYPE_INTERNAL, FeedbackMessage::ID_GROUPLIST_UPDATED, bridge);
    EXPECT_CALL(*_mockConnectionMonitor, Start(bridge, 5000));
    _mockConnect->callback(message);
}

TEST_F(TestHuestream, NewFeedbackMessage_TYPE_INTERNAL_stops_monitor_if_not_connectable) {
    auto bridge = std::make_shared<Bridge>(std::make_shared<BridgeSettings>());
    auto message = FeedbackMessage(FeedbackMessage::REQUEST_TYPE_INTERNAL, FeedbackMessage::ID_STREAMING_DISCONNECTED, bridge);
    EXPECT_CALL(*_mockConnectionMonitor, Stop());
    _mockConnect->callback(message);
}

TEST_F(TestHuestream, NewFeedbackMessage_PROCEDURE_FINISHED_starts_monitor_if_connectable) {
    auto bridge = CreateNotStreamingBridge();
    auto message = FeedbackMessage(FeedbackMessage::REQUEST_TYPE_CONNECT, FeedbackMessage::ID_USERPROCEDURE_FINISHED, bridge);
    EXPECT_CALL(*_mockConnectionMonitor, Start(bridge, 15000));
    _mockConnect->callback(message);
}

TEST_F(TestHuestream, NewFeedbackMessage_PROCEDURE_FINISHED_starts_monitor_if_streaming) {
    auto bridge = CreateStreamingBridge();
    auto message = FeedbackMessage(FeedbackMessage::REQUEST_TYPE_CONNECT, FeedbackMessage::ID_USERPROCEDURE_FINISHED, bridge);
    EXPECT_CALL(*_mockConnectionMonitor, Start(bridge, 5000));
    _mockConnect->callback(message);
}

TEST_F(TestHuestream, NewFeedbackMessage_PROCEDURE_FINISHED_stops_monitor_if_not_connectable) {
    auto bridge = std::make_shared<Bridge>(std::make_shared<BridgeSettings>());
    auto message = FeedbackMessage(FeedbackMessage::REQUEST_TYPE_CONNECT, FeedbackMessage::ID_USERPROCEDURE_FINISHED, bridge);
    EXPECT_CALL(*_mockConnectionMonitor, Stop());
    _mockConnect->callback(message);
}

TEST_F(TestHuestream, NewFeedbackMessage_PROCEDURE_STARTED_stops_monitor) {
    auto bridge = CreateNotStreamingBridge();
    auto message = FeedbackMessage(FeedbackMessage::REQUEST_TYPE_ACTIVATE, FeedbackMessage::ID_USERPROCEDURE_STARTED, bridge);
    EXPECT_CALL(*_mockConnectionMonitor, Stop());
    _mockConnect->callback(message);
}

TEST_F(TestHuestream, NewFeedbackMessage_PROCEDURE_FINISHED_passes_bridge_to_group_controller) {
    auto bridge = std::make_shared<Bridge>(std::make_shared<BridgeSettings>());
    auto message = FeedbackMessage(FeedbackMessage::REQUEST_TYPE_CONNECT, FeedbackMessage::ID_USERPROCEDURE_FINISHED, bridge);
    EXPECT_CALL(*_mockGroupController, SetActiveBridge(_));
    _mockConnect->callback(message);
}

TEST_F(TestHuestream, NewFeedbackMessage_TYPE_INTERNAL_passes_bridge_to_group_controller) {
    auto bridge = std::make_shared<Bridge>(std::make_shared<BridgeSettings>());
    auto message = FeedbackMessage(FeedbackMessage::REQUEST_TYPE_INTERNAL, FeedbackMessage::ID_GROUPLIST_UPDATED, bridge);
    EXPECT_CALL(*_mockGroupController, SetActiveBridge(_));
    _mockConnect->callback(message);
}

TEST_F(TestHuestream, NewFeedbackMessage_FINISH_LOADING_START_SAVING_stores_known_bridges) {
    ASSERT_EQ(huestream->GetKnownBridges()->size(), 0);

    auto bridge = std::make_shared<Bridge>(std::make_shared<BridgeSettings>());
    auto bridges = std::make_shared<BridgeList>();
    bridges->push_back(bridge);
    auto message = FeedbackMessage(FeedbackMessage::REQUEST_TYPE_LOAD_BRIDGE, FeedbackMessage::ID_FINISH_LOADING_BRIDGE_CONFIGURED, bridge, bridges);
    _mockConnect->callback(message);
    ASSERT_EQ(huestream->GetKnownBridges()->size(), 1);

    bridges = std::make_shared<BridgeList>();
    message = FeedbackMessage(FeedbackMessage::REQUEST_TYPE_RESET_ALL, FeedbackMessage::ID_START_SAVING, bridge, bridges);
    _mockConnect->callback(message);
    ASSERT_EQ(huestream->GetKnownBridges()->size(), 0);
}

TEST_F(TestHuestream, SetOn) {
    EXPECT_CALL(*_mockGroupController, SetOn(true));
    huestream->SetGroupOn(true);
}

TEST_F(TestHuestream, SetBrightness) {
    EXPECT_CALL(*_mockGroupController, SetBrightness(0.3));
    huestream->SetGroupBrightness(0.3);
}

TEST_F(TestHuestream, SetColor) {
    EXPECT_CALL(*_mockGroupController, SetColor(0.8, 0.4));
    huestream->SetGroupColor(0.8, 0.4);
}

TEST_F(TestHuestream, SetPresetDefault) {
    EXPECT_CALL(*_mockGroupController, SetPreset(BasicGroupLightController::LIGHT_PRESET_RELAX, false));
    huestream->SetGroupPreset(BasicGroupLightController::LIGHT_PRESET_RELAX);
}

TEST_F(TestHuestream, SetPresetCustom) {
    EXPECT_CALL(*_mockGroupController, SetPreset(0.1, 0.2, 0.3, false));
    huestream->SetGroupPreset(0.1, 0.2, 0.3);
}

TEST_F(TestHuestream, SetScene) {
    EXPECT_CALL(*_mockGroupController, SetScene("12ab00ffgg"));
    huestream->SetGroupScene("12ab00ffgg");
}

TEST_F(TestHuestream, LightStateUpdatedHandler) {
    huestream->RegisterLightStateUpdatedHandler(_handler);
    auto bridge = std::make_shared<Bridge>(std::make_shared<BridgeSettings>());
    auto group = std::make_shared<Group>();
    group->SetId("2");
    bridge->GetGroups()->push_back(group);
    bridge->SelectGroup("2");
    huestream->SetActiveBridge(bridge);

    EXPECT_CALL(*_mockMixer, Lock());
    EXPECT_CALL(*_mockMixer, Render());
    EXPECT_CALL(*_mockMixer, Unlock());

    ASSERT_TRUE(_handler->_lights == nullptr);
    _mockStream->ExecuteRenderCallback();
    ASSERT_TRUE(_handler->_lights != nullptr);
}

/******************************************************************************/
/*                                 END OF FILE                                */
/******************************************************************************/