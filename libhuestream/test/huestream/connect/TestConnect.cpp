/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>
#include "huestream/connect/Connect.h"
#include "test/huestream/_mock/MockFeedbackMessageHandler.h"
#include "test/huestream/_mock/MockBridgeHttpClient.h"
#include "test/huestream/_mock/MockMessageDispatcher.h"
#include "test/huestream/_mock/MockStream.h"
#include "test/huestream/_mock/MockBridgeStorageAccessor.h"
#include "test/huestream/_mock/MockBridge.h"
#include "test/huestream/_mock/MockScheduler.h"
#include "support/scheduler/SchedulerFactory.h"

namespace huestream {
    class TestConnect : public ::testing::Test {
    public:
        void SetUp() override {
            _scheduler = std::make_shared<NiceMock<MockScheduler>>();
            support::SchedulerFactory::set_factory_method([this](unsigned int interval) {
                return std::unique_ptr<MockWrapperScheduler>(new MockWrapperScheduler(interval, _scheduler));
            });
            _httpClient = std::make_shared<MockBridgeHttpClient>();
            _messageDispatcher = std::make_shared<MockMessageDispatcher>();
            _stream = std::make_shared<MockStream>();
            _bridgeStorageAccessor = std::make_shared<MockBridgeStorageAccessor>();
            _connect = std::make_shared<Connect>(_httpClient, _messageDispatcher, std::make_shared<BridgeSettings>(), std::make_shared<AppSettings>(), _stream, _bridgeStorageAccessor);
            _bridge = std::make_shared<MockBridge>(std::make_shared<BridgeSettings>());
            _feedbackMessageHandler = std::make_shared<MockFeedbackMessageHandler>();
            _connect->SetFeedbackHandler(_feedbackMessageHandler);
        }

        void TearDown() override {

        }

    protected:
        std::shared_ptr<Connect> _connect;
        std::shared_ptr<MockBridgeHttpClient> _httpClient;
        std::shared_ptr<MockMessageDispatcher> _messageDispatcher;
        std::shared_ptr<MockStream> _stream;
        std::shared_ptr<MockBridgeStorageAccessor> _bridgeStorageAccessor;
        std::shared_ptr<MockFeedbackMessageHandler> _feedbackMessageHandler;
        std::shared_ptr<MockBridge> _bridge;
        std::shared_ptr<MockScheduler> _scheduler;
    };

    TEST_F(TestConnect, NewMessageFeedbackHandler) {
        FeedbackMessage message(FeedbackMessage::REQUEST_TYPE_INTERNAL, FeedbackMessage::ID_USERPROCEDURE_STARTED, _bridge);

        EXPECT_CALL(*_feedbackMessageHandler, NewFeedbackMessage(_));
        _connect->NewMessage(message);
    }

    TEST_F(TestConnect, NewMessageResults) {
        std::vector<std::tuple<FeedbackMessage::Id, BridgeStatus, ConnectResult>> idStatusResultMap = {
            std::make_tuple(FeedbackMessage::ID_DONE_RESET,             BRIDGE_EMPTY,                  Uninitialized),
            std::make_tuple(FeedbackMessage::ID_DONE_NO_BRIDGE_FOUND,   BRIDGE_EMPTY,                  NoBridgeFound),
            std::make_tuple(FeedbackMessage::ID_DONE_BRIDGE_FOUND,      BRIDGE_NOT_CONNECTABLE,        BridgeFound),
            std::make_tuple(FeedbackMessage::ID_DONE_ACTION_REQUIRED,   BRIDGE_NOT_CONNECTED,          ActionRequired),
            std::make_tuple(FeedbackMessage::ID_DONE_ACTION_REQUIRED,   BRIDGE_INVALID_VERSION,        ActionRequired),
            std::make_tuple(FeedbackMessage::ID_DONE_ACTION_REQUIRED,   BRIDGE_NO_GROUP_AVAILABLE,     ActionRequired),
            std::make_tuple(FeedbackMessage::ID_DONE_ACTION_REQUIRED,   BRIDGE_BUSY,                   ActionRequired),
            std::make_tuple(FeedbackMessage::ID_DONE_COMPLETED,         BRIDGE_READY,                  ReadyToStart),
            std::make_tuple(FeedbackMessage::ID_DONE_COMPLETED,         BRIDGE_STREAMING,              Streaming),
            std::make_tuple(FeedbackMessage::ID_BRIDGE_DISCONNECTED,    BRIDGE_NOT_CONNECTED,          ActionRequired),
            std::make_tuple(FeedbackMessage::ID_STREAMING_DISCONNECTED, BRIDGE_READY,                  ReadyToStart),
            std::make_tuple(FeedbackMessage::ID_STREAMING_DISCONNECTED, BRIDGE_INVALID_GROUP_SELECTED, ActionRequired)
        };

        EXPECT_CALL(*_feedbackMessageHandler, NewFeedbackMessage(_)).Times(idStatusResultMap.size());

        FeedbackMessage message(FeedbackMessage::REQUEST_TYPE_INTERNAL, FeedbackMessage::ID_USERPROCEDURE_STARTED, _bridge);

        for (auto& idResultPair : idStatusResultMap) {
            message.SetId(std::get<0>(idResultPair));
            EXPECT_CALL(*_bridge, GetStatus()).Times(AtLeast(0)).WillOnce(Return(std::get<1>(idResultPair)));
            _connect->NewMessage(message);
            EXPECT_EQ(_connect->GetResult(), std::get<2>(idResultPair));
        }
    }

    TEST_F(TestConnect, IsReady) {
        EXPECT_CALL(*_feedbackMessageHandler, NewFeedbackMessage(_));
        _connect->NewMessage(FeedbackMessage(FeedbackMessage::REQUEST_TYPE_INTERNAL, FeedbackMessage::ID_USERPROCEDURE_STARTED, _bridge));
        ASSERT_FALSE(_connect->IsReady());

        EXPECT_CALL(*_feedbackMessageHandler, NewFeedbackMessage(_));
        _connect->NewMessage(FeedbackMessage(FeedbackMessage::REQUEST_TYPE_INTERNAL, FeedbackMessage::ID_USERPROCEDURE_FINISHED, _bridge));
        ASSERT_TRUE(_connect->IsReady());
    }

    TEST_F(TestConnect, ConnectToBridge) {
        EXPECT_CALL(*_messageDispatcher, Queue(_));
        ASSERT_TRUE(_connect->IsReady());
        _connect->ConnectToBridge();
        ASSERT_FALSE(_connect->IsReady());
    }

    TEST_F(TestConnect, ConnectToBridgeBackground) {
        EXPECT_CALL(*_messageDispatcher, Queue(_));
        ASSERT_TRUE(_connect->IsReady());
        _connect->ConnectToBridgeBackground();
        ASSERT_FALSE(_connect->IsReady());
    }

    TEST_F(TestConnect, ConnectToBridgeWithIp) {
        EXPECT_CALL(*_messageDispatcher, Queue(_));
        ASSERT_TRUE(_connect->IsReady());
        _connect->ConnectToBridgeWithIp("0.0.0.0");
        ASSERT_FALSE(_connect->IsReady());
    }

    TEST_F(TestConnect, ConnectToNewBridge) {
        EXPECT_CALL(*_messageDispatcher, Queue(_));
        ASSERT_TRUE(_connect->IsReady());
        _connect->ConnectToNewBridge();
        ASSERT_FALSE(_connect->IsReady());
    }

    TEST_F(TestConnect, SetManual) {
        EXPECT_CALL(*_messageDispatcher, Queue(_));
        ASSERT_TRUE(_connect->IsReady());
        _connect->SetManual(nullptr);
        ASSERT_FALSE(_connect->IsReady());
    }

    TEST_F(TestConnect, Abort) {
        EXPECT_CALL(*_messageDispatcher, Queue(_));
        ASSERT_TRUE(_connect->IsReady());
        _connect->Abort();
        ASSERT_TRUE(_connect->IsReady());
    }

    TEST_F(TestConnect, ResetBridge) {
        EXPECT_CALL(*_messageDispatcher, Queue(_));
        ASSERT_TRUE(_connect->IsReady());
        _connect->ResetBridge();
        ASSERT_FALSE(_connect->IsReady());
    }

    TEST_F(TestConnect, ResetAllData) {
        EXPECT_CALL(*_messageDispatcher, Queue(_));
        ASSERT_TRUE(_connect->IsReady());
        _connect->ResetAllData();
        ASSERT_FALSE(_connect->IsReady());
    }

    TEST_F(TestConnect, SelectGroup) {
        EXPECT_CALL(*_messageDispatcher, Queue(_));
        ASSERT_TRUE(_connect->IsReady());
        _connect->SelectGroup("0");
        ASSERT_FALSE(_connect->IsReady());
    }

    TEST_F(TestConnect, Load) {
        EXPECT_CALL(*_messageDispatcher, Queue(_));
        ASSERT_TRUE(_connect->IsReady());
        _connect->Load();
        ASSERT_FALSE(_connect->IsReady());
    }

    TEST_F(TestConnect, StartStream) {
        EXPECT_CALL(*_messageDispatcher, Queue(_));
        ASSERT_TRUE(_connect->IsReady());
        _connect->StartStream(nullptr);
        // Here we don't check _connect->IsReady() because it won't be false. It will only be set to false asynchronously.
    }

    TEST_F(TestConnect, StopStream) {
        EXPECT_CALL(*_messageDispatcher, Queue(_));
        ASSERT_TRUE(_connect->IsReady());
        _connect->StopStream(nullptr);
        // Here we don't check _connect->IsReady() because it won't be false. It will only be set to false asynchronously.
    }

    TEST_F(TestConnect, OnBridgeMonitorEvent) {
        EXPECT_CALL(*_messageDispatcher, Queue(_));
        ASSERT_TRUE(_connect->IsReady());
        _connect->OnBridgeMonitorEvent(FeedbackMessage(
            FeedbackMessage::REQUEST_TYPE_INTERNAL, FeedbackMessage::ID_USERPROCEDURE_STARTED,
            std::make_shared<Bridge>(std::make_shared<BridgeSettings>())));
        ASSERT_TRUE(_connect->IsReady());
    }
}
