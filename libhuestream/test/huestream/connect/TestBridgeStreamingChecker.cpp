#include <huestream/connect/BridgeStreamingChecker.h>
#include <test/huestream/_mock/MockConfigRetriever.h>
#include <test/huestream/_mock/MockBridgeHttpClient.h>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

using ::testing::Return;
using ::testing::_;
using ::testing::Sequence;
using ::testing::Property;
using ::testing::AllOf;
using ::testing::Pointee;

namespace huestream {
class TestBridgeStreamingChecker : public testing::Test {
 public:
  virtual void SetUp() {
      _messages = std::vector<FeedbackMessage>();
      InitTestData();
      InitBridgeStateChecker();
  }

  virtual void TearDown() {
      _messages.clear();
  }

  void InitBridgeStateChecker() {
      _httpClient = std::make_shared<MockBridgeHttpClient>();
      _mockFullConfigRetriever = std::shared_ptr<MockConfigRetriever>(new MockConfigRetriever());
      _bridgeStreamingChecker = std::make_shared<BridgeStreamingChecker>(std::static_pointer_cast<IFullConfigRetriever>(_mockFullConfigRetriever), _httpClient);
      _bridgeStreamingChecker->SetFeedbackMessageCallback([this](const huestream::FeedbackMessage &message){
        _messages.push_back(message);
      });
  }
  void InitTestData() {
      _bridgeSettings = std::make_shared<BridgeSettings>();
      _currentBridge = std::make_shared<Bridge>(_bridgeSettings);
      InitStreamableBridge(_currentBridge);
      EXPECT_EQ(2, _currentBridge->GetGroups()->size());

      _actualBridge = std::make_shared<Bridge>(_bridgeSettings);
      InitStreamableBridge(_actualBridge);
  }

  static void InitStreamableBridge(BridgePtr bridge) {
      bridge->SetId("00:12:12");
      bridge->SetModelId("BSB002");
      bridge->SetApiversion("1.24.0");
      bridge->SetSwversion("1940094000");
      bridge->SetIpAddress("123.123.124.125");
      bridge->SetClientKey("12345678901234567890123456789012");
      bridge->SetIsAuthorized(true);
      bridge->SetIsValidIp(true);
      bridge->SetUser("SOME_USER");
      bridge->SetSelectedGroup("1");

      auto group1 = std::make_shared<Group>();
      InitGroup(group1, "1");
      group1->SetActive(true);
      group1->SetOwner("SOME_USER");
      group1->SetOnState(true);
      group1->SetBrightnessState(0.8);
      bridge->GetGroups()->push_back(group1);

      auto group2 = std::make_shared<Group>();
      InitGroup(group2, "2");
      group2->SetActive(false);
      group2->SetOwner("");
      bridge->GetGroups()->push_back(group2);
  }

  static void InitGroup(GroupPtr group, std::string nr) {
      group->SetId(nr);
      group->SetName("group " + nr);
      group->AddLight("1", 0.1, 0.1, 0.1, "L1", "LTC001");
      group->AddLight("2", 0.2, 0.2, 0.2, "L2", "LTC001");
  }

  void Expect_FullConfigRetriever_Execute_Once() {

      EXPECT_CALL(*_mockFullConfigRetriever, Execute(_, _, _)).Times(1)
          .WillRepeatedly(DoAll(
              SaveArg<0>(&_mockFullConfigRetriever->Bridge),
              SaveArg<1>(&_mockFullConfigRetriever->RetrieveCallback),
							SaveArg<2>(&_mockFullConfigRetriever->Feedback),
              Return(true)));
  }

  void ExpectOneMessageOfType(FeedbackMessage::Id id) {
      ASSERT_EQ(_messages.size(), 1);
      EXPECT_EQ(_messages[0].GetRequestType(), FeedbackMessage::REQUEST_TYPE_INTERNAL);
      EXPECT_EQ(_messages[0].GetId(), id);
  }

  void ExpectTwoMessagesOfType(FeedbackMessage::Id id0, FeedbackMessage::Id id1) {
      ASSERT_EQ(_messages.size(), 2);
      EXPECT_EQ(_messages[0].GetRequestType(), FeedbackMessage::REQUEST_TYPE_INTERNAL);
      EXPECT_EQ(_messages[0].GetId(), id0);
      EXPECT_EQ(_messages[1].GetRequestType(), FeedbackMessage::REQUEST_TYPE_INTERNAL);
      EXPECT_EQ(_messages[1].GetId(), id1);
  }

  void ExpectThreeMessagesOfType(FeedbackMessage::Id id0, FeedbackMessage::Id id1, FeedbackMessage::Id id2) {
      ASSERT_EQ(_messages.size(), 3);
      EXPECT_EQ(_messages[0].GetRequestType(), FeedbackMessage::REQUEST_TYPE_INTERNAL);
      EXPECT_EQ(_messages[0].GetId(), id0);
      EXPECT_EQ(_messages[1].GetRequestType(), FeedbackMessage::REQUEST_TYPE_INTERNAL);
      EXPECT_EQ(_messages[1].GetId(), id1);
      EXPECT_EQ(_messages[2].GetRequestType(), FeedbackMessage::REQUEST_TYPE_INTERNAL);
      EXPECT_EQ(_messages[2].GetId(), id2);
  }

  void ExecuteCheckAndExpectOneFeedbackMessageOfType(FeedbackMessage::Id id) {
      Expect_FullConfigRetriever_Execute_Once();
      _bridgeStreamingChecker->Check(_currentBridge);
      _mockFullConfigRetriever->ExecuteRetrieveCallback(OPERATION_SUCCESS, _actualBridge);
      ExpectOneMessageOfType(id);
  }

  void ExecuteCheckAndExpectTwoFeedbackMessagesOfType(FeedbackMessage::Id id0, FeedbackMessage::Id id1) {
      Expect_FullConfigRetriever_Execute_Once();
      _bridgeStreamingChecker->Check(_currentBridge);
      _mockFullConfigRetriever->ExecuteRetrieveCallback(OPERATION_SUCCESS, _actualBridge);
      ExpectTwoMessagesOfType(id0, id1);
  }

  void ExecuteCheckAndExpectThreeFeedbackMessagesOfType(FeedbackMessage::Id id0, FeedbackMessage::Id id1, FeedbackMessage::Id id2) {
      Expect_FullConfigRetriever_Execute_Once();
      _bridgeStreamingChecker->Check(_currentBridge);
      _mockFullConfigRetriever->ExecuteRetrieveCallback(OPERATION_SUCCESS, _actualBridge);
      ExpectThreeMessagesOfType(id0, id1, id2);
  }

  void Expect_FullConfigRetriever_Execute_Never() {
      EXPECT_CALL(*_mockFullConfigRetriever, Execute(_, _, _)).Times(0);
  }

  BridgePtr _currentBridge;
  BridgePtr _actualBridge;
  BridgeSettingsPtr _bridgeSettings;
  std::shared_ptr<MockConfigRetriever> _mockFullConfigRetriever;
	std::shared_ptr<MockBridgeHttpClient> _httpClient;
  std::shared_ptr<BridgeStreamingChecker> _bridgeStreamingChecker;
  std::vector<FeedbackMessage> _messages;

};

TEST_F(TestBridgeStreamingChecker, WhenCurrentStateBridgeNotConnectable_CheckDoesNotPerformACheck) {
    _currentBridge->SetIsAuthorized(false);
    Expect_FullConfigRetriever_Execute_Never();
    _bridgeStreamingChecker->Check(_currentBridge);
}

TEST_F(TestBridgeStreamingChecker, WhenCurrentStateBridgeNotStreaming_NoStreamingDisconnectedFeedbackMessage) {
    _currentBridge->GetGroups()->at(0)->SetActive(false);
    _actualBridge->GetGroups()->at(0)->SetActive(false);
    Expect_FullConfigRetriever_Execute_Once();
    _bridgeStreamingChecker->Check(_currentBridge);
    EXPECT_EQ(_messages.size(), 0);
}

TEST_F(TestBridgeStreamingChecker, WhenNothingChanges_NoFeedbackMessage) {
    Expect_FullConfigRetriever_Execute_Once();
    _bridgeStreamingChecker->Check(_currentBridge);
    _mockFullConfigRetriever->ExecuteRetrieveCallback(OPERATION_SUCCESS, _actualBridge);
    EXPECT_EQ(_messages.size(), 0);
}

TEST_F(TestBridgeStreamingChecker, WhenNotSelectedGroupChanges_NoFeedbackMessage) {
    _actualBridge->GetGroups()->at(1)->AddLight("9", 0.9, 0.9, 0.9, "", "");
    Expect_FullConfigRetriever_Execute_Once();
    _bridgeStreamingChecker->Check(_currentBridge);
    _mockFullConfigRetriever->ExecuteRetrieveCallback(OPERATION_SUCCESS, _actualBridge);
    EXPECT_EQ(_messages.size(), 0);
}

TEST_F(TestBridgeStreamingChecker, WhenRetrieverFails_BRIDGE_DISCONNECTED) {
    _currentBridge->GetGroup()->SetActive(false);
    _currentBridge->GetGroup()->SetOwner("");
    _actualBridge->GetGroup()->SetActive(false);
    _actualBridge->GetGroup()->SetOwner("");
    _actualBridge->SetIsValidIp(false);
    Expect_FullConfigRetriever_Execute_Once();
    _bridgeStreamingChecker->Check(_currentBridge);
    _mockFullConfigRetriever->ExecuteRetrieveCallback(OPERATION_FAILED, _actualBridge);
    ExpectOneMessageOfType(FeedbackMessage::ID_BRIDGE_DISCONNECTED);
}

TEST_F(TestBridgeStreamingChecker, WhenRetrieverFails_STREAMING_DISCONNECTED_BRIDGE_DISCONNECTED) {
    _actualBridge->SetIsValidIp(false);
    Expect_FullConfigRetriever_Execute_Once();
    _bridgeStreamingChecker->Check(_currentBridge);
    _mockFullConfigRetriever->ExecuteRetrieveCallback(OPERATION_FAILED, _actualBridge);
    ExpectTwoMessagesOfType(FeedbackMessage::ID_BRIDGE_DISCONNECTED, FeedbackMessage::ID_STREAMING_DISCONNECTED);
}

TEST_F(TestBridgeStreamingChecker, WhenCurrentGroupDeleted_STREAMING_DISCONNECTED) {
    _actualBridge->DeleteGroup("1");
    ExecuteCheckAndExpectThreeFeedbackMessagesOfType(FeedbackMessage::ID_STREAMING_DISCONNECTED,
        FeedbackMessage::ID_GROUPLIST_UPDATED, FeedbackMessage::ID_LIGHTS_UPDATED);
}

TEST_F(TestBridgeStreamingChecker, WhenRetrieverActiveFalse_STREAMING_DISCONNECTED) {
    _actualBridge->GetGroups()->at(0)->SetActive(false);
    ExecuteCheckAndExpectTwoFeedbackMessagesOfType(FeedbackMessage::ID_STREAMING_DISCONNECTED, FeedbackMessage::ID_GROUPLIST_UPDATED);
}

TEST_F(TestBridgeStreamingChecker, WhenRetrieverOtherUser_STREAMING_DISCONNECTED) {
    _actualBridge->GetGroups()->at(0)->SetOwner("OTHER_USER");
    ExecuteCheckAndExpectTwoFeedbackMessagesOfType(FeedbackMessage::ID_STREAMING_DISCONNECTED, FeedbackMessage::ID_GROUPLIST_UPDATED);
}

TEST_F(TestBridgeStreamingChecker, WhenRetrieverConnected_BRIDGE_CONNECTED) {
    _currentBridge->SetIsValidIp(false);
    _currentBridge->GetGroups()->at(0)->SetActive(false);
    _actualBridge->GetGroups()->at(0)->SetActive(false);
    ExecuteCheckAndExpectOneFeedbackMessageOfType(FeedbackMessage::ID_BRIDGE_CONNECTED);
}

TEST_F(TestBridgeStreamingChecker, WhenRetrieverLightRemoved_LIGHTS_UPDATED) {
    _actualBridge->GetGroups()->at(0)->GetLights()->pop_back();
    ExecuteCheckAndExpectOneFeedbackMessageOfType(FeedbackMessage::ID_LIGHTS_UPDATED);
}

TEST_F(TestBridgeStreamingChecker, WhenRetrieverLightAdded_LIGHTS_UPDATED) {
    _actualBridge->GetGroups()->at(0)->AddLight("4", 0.4, 0.4, 0.4, "", "");
    ExecuteCheckAndExpectOneFeedbackMessageOfType(FeedbackMessage::ID_LIGHTS_UPDATED);
}

TEST_F(TestBridgeStreamingChecker, WhenRetrieverLightChangedLocationX_LIGHTS_UPDATED) {
    _actualBridge->GetGroups()->at(0)->GetLights()->at(0)->SetPosition(Location(0.2,0.1));
    ExecuteCheckAndExpectOneFeedbackMessageOfType(FeedbackMessage::ID_LIGHTS_UPDATED);
}

TEST_F(TestBridgeStreamingChecker, WhenRetrieverLightChangedLocationY_LIGHTS_UPDATED) {
    _actualBridge->GetGroups()->at(0)->GetLights()->at(0)->SetPosition(Location(0.1,0.2));
    ExecuteCheckAndExpectOneFeedbackMessageOfType(FeedbackMessage::ID_LIGHTS_UPDATED);
}

TEST_F(TestBridgeStreamingChecker, WhenRetrieverLightOrderChanges_LIGHTS_UPDATED) {
    LightListPtr newLights = std::make_shared<LightList>();
    newLights->push_back(_actualBridge->GetGroups()->at(0)->GetLights()->at(1));
    newLights->push_back(_actualBridge->GetGroups()->at(0)->GetLights()->at(0));
    _actualBridge->GetGroups()->at(0)->GetLights()->clear();
    _actualBridge->GetGroups()->at(0)->SetLights(newLights);
    ExecuteCheckAndExpectOneFeedbackMessageOfType(FeedbackMessage::ID_LIGHTS_UPDATED);
}

TEST_F(TestBridgeStreamingChecker, WhenRetrieverGroupSelected_LIGHTS_UPDATED_GROUP_LIGHTSTATE_UPDATED) {
    _currentBridge->GetGroups()->at(0)->SetActive(false);
    _currentBridge->GetGroups()->at(0)->SetOwner("");
    _currentBridge->SetSelectedGroup("");
    _actualBridge->GetGroups()->at(0)->SetActive(false);
    _actualBridge->GetGroups()->at(0)->SetOwner("");
    ExecuteCheckAndExpectTwoFeedbackMessagesOfType(FeedbackMessage::ID_LIGHTS_UPDATED,
        FeedbackMessage::ID_GROUP_LIGHTSTATE_UPDATED);
}

TEST_F(TestBridgeStreamingChecker, WhenRetrieverGroupSelectedAndStreaming_STREAMING_CONNECTED_GROUPLIST_UPDATED_LIGHTS_UPDATED) {
    _currentBridge->GetGroups()->at(0)->SetActive(false);
    _currentBridge->GetGroups()->at(0)->SetOwner("");
    _currentBridge->SetSelectedGroup("");
    ExecuteCheckAndExpectThreeFeedbackMessagesOfType(FeedbackMessage::ID_STREAMING_CONNECTED,
        FeedbackMessage::ID_GROUPLIST_UPDATED, FeedbackMessage::ID_LIGHTS_UPDATED);
}

TEST_F(TestBridgeStreamingChecker, WhenRetrieverGroupRemoved_GROUPLIST_UPDATED) {
    _actualBridge->DeleteGroup("2");
    ExecuteCheckAndExpectOneFeedbackMessageOfType(FeedbackMessage::ID_GROUPLIST_UPDATED);
}

TEST_F(TestBridgeStreamingChecker, WhenRetrieverGroupAdded_GROUPLIST_UPDATED) {
    _actualBridge->GetGroups()->push_back(std::make_shared<Group>());
    ExecuteCheckAndExpectOneFeedbackMessageOfType(FeedbackMessage::ID_GROUPLIST_UPDATED);
}

TEST_F(TestBridgeStreamingChecker, WhenRetrieverGroupAddedAndRemoved_GROUPLIST_UPDATED) {
    _actualBridge->GetGroups()->at(1)->SetId("3");
    ExecuteCheckAndExpectOneFeedbackMessageOfType(FeedbackMessage::ID_GROUPLIST_UPDATED);
}

TEST_F(TestBridgeStreamingChecker, WhenRetrieverGroupNameChanged_GROUPLIST_UPDATED) {
    _actualBridge->GetGroups()->at(1)->SetName("newname");
    ExecuteCheckAndExpectOneFeedbackMessageOfType(FeedbackMessage::ID_GROUPLIST_UPDATED);
}

TEST_F(TestBridgeStreamingChecker, WhenRetrieverOtherGroupActiveChanged_GROUPLIST_UPDATED) {
    _currentBridge->GetGroups()->at(0)->SetActive(false);
    _currentBridge->GetGroups()->at(0)->SetOwner("");
    _actualBridge->GetGroups()->at(0)->SetActive(false);
    _actualBridge->GetGroups()->at(0)->SetOwner("");
    _actualBridge->GetGroups()->at(1)->SetActive(true);
    _actualBridge->GetGroups()->at(1)->SetOwner("test");
    ExecuteCheckAndExpectOneFeedbackMessageOfType(FeedbackMessage::ID_GROUPLIST_UPDATED);
}

TEST_F(TestBridgeStreamingChecker, WhenRetrieverOtherGroupOwnerChanged_GROUPLIST_UPDATED) {
    _currentBridge->GetGroups()->at(0)->SetActive(false);
    _currentBridge->GetGroups()->at(0)->SetOwner("");
    _currentBridge->GetGroups()->at(1)->SetActive(true);
    _currentBridge->GetGroups()->at(1)->SetOwner("aUser");
    _actualBridge->GetGroups()->at(0)->SetActive(false);
    _actualBridge->GetGroups()->at(0)->SetOwner("");
    _actualBridge->GetGroups()->at(1)->SetActive(true);
    _actualBridge->GetGroups()->at(1)->SetOwner("anOtherUser");
    ExecuteCheckAndExpectOneFeedbackMessageOfType(FeedbackMessage::ID_GROUPLIST_UPDATED);
}

TEST_F(TestBridgeStreamingChecker, WhenRetrieverLightAndGroupAdded_Two_Messages) {
    _actualBridge->GetGroups()->at(0)->AddLight("4", 0.4, 0.4, 0.4, "", "");
    _actualBridge->GetGroups()->push_back(std::make_shared<Group>());
    ExecuteCheckAndExpectTwoFeedbackMessagesOfType(FeedbackMessage::ID_GROUPLIST_UPDATED,
        FeedbackMessage::ID_LIGHTS_UPDATED);
}

TEST_F(TestBridgeStreamingChecker, WhenRetrieverLightAndGroupAddedAndConnected_Three_Messages) {
    _currentBridge->SetIsValidIp(false);
    _currentBridge->GetGroups()->at(0)->SetActive(false);
    _actualBridge->GetGroups()->at(0)->SetOwner("anOtherUser");
    _actualBridge->GetGroups()->at(0)->AddLight("4", 0.4, 0.4, 0.4, "", "");
    _actualBridge->GetGroups()->push_back(std::make_shared<Group>());
    ExecuteCheckAndExpectThreeFeedbackMessagesOfType(FeedbackMessage::ID_BRIDGE_CONNECTED,
        FeedbackMessage::ID_GROUPLIST_UPDATED, FeedbackMessage::ID_LIGHTS_UPDATED);
}

TEST_F(TestBridgeStreamingChecker, WhenRetrieverGroupOnStateChanged_GROUP_LIGHTSTATE_UPDATED) {
    _currentBridge->GetGroup()->SetActive(false);
    _actualBridge->GetGroup()->SetActive(false);
    _actualBridge->GetGroups()->at(0)->SetOnState(false);
    ExecuteCheckAndExpectOneFeedbackMessageOfType(FeedbackMessage::ID_GROUP_LIGHTSTATE_UPDATED);
}

TEST_F(TestBridgeStreamingChecker, WhenRetrieverGroupBrightnessStateChanged_GROUP_LIGHTSTATE_UPDATED) {
    _currentBridge->GetGroup()->SetActive(false);
    _actualBridge->GetGroup()->SetActive(false);
    _actualBridge->GetGroups()->at(0)->SetBrightnessState(0.1);
    ExecuteCheckAndExpectOneFeedbackMessageOfType(FeedbackMessage::ID_GROUP_LIGHTSTATE_UPDATED);
}

TEST_F(TestBridgeStreamingChecker, WhenStreaming_NoGroupStateMessages) {
    _actualBridge->GetGroups()->at(0)->SetOnState(false);
    Expect_FullConfigRetriever_Execute_Once();
    _bridgeStreamingChecker->Check(_currentBridge);
    _mockFullConfigRetriever->ExecuteRetrieveCallback(OPERATION_SUCCESS, _actualBridge);
    EXPECT_EQ(_messages.size(), 0);
}

TEST_F(TestBridgeStreamingChecker, WhenRetrieverOtherGroupBrightnessStateChanged_NoFeedbackMessage) {
    _currentBridge->GetGroup()->SetActive(false);
    _actualBridge->GetGroup()->SetActive(false);
    _actualBridge->GetGroups()->at(1)->SetBrightnessState(0.123);
    Expect_FullConfigRetriever_Execute_Once();
    _bridgeStreamingChecker->Check(_currentBridge);
    _mockFullConfigRetriever->ExecuteRetrieveCallback(OPERATION_SUCCESS, _actualBridge);
    EXPECT_EQ(_messages.size(), 0);
}

}  // namespace huestream
