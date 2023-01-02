#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <memory>
#include <huestream/connect/ConnectionFlow.h>
#include "test/huestream/_mock/MockBridgeAuthenticator.h"
#include "test/huestream/_mock/MockBridgeSearcher.h"
#include "TestableConnectionFlow.h"
#include "test/huestream/_mock/MockConnectionFlowFactory.h"
#include "test/huestream/_mock/MockBridgeStorageAccessor.h"
#include "test/huestream/_stub/StubMessageDispatcher.h"
#include "TestConnectionFlowBase.h"

using namespace testing;
using namespace huestream;


class TestConnectionFlow_Manual : public TestConnectionFlowBase {
public:

    void connect_manual_ip_load_retrieve_push_link(int index) {
        expect_message(FeedbackMessage::ID_USERPROCEDURE_STARTED, FeedbackMessage::FEEDBACK_TYPE_INFO);

        expect_storage_accessor_load_return_data();
        expect_small_config_retrieval_return_data(index);
        expect_message(FeedbackMessage::ID_START_AUTHORIZING, FeedbackMessage::FEEDBACK_TYPE_INFO);
        expect_message(FeedbackMessage::ID_PRESS_PUSH_LINK, FeedbackMessage::FEEDBACK_TYPE_USER);
        expect_on_authenticator_authenticate(index);

        std::string ipAddress = _bridges->at(index)->GetIpAddress();
        _connectionFlow->ConnectToBridgeWithIp(ipAddress);
        _messageDispatcher->ExecutePendingActions();
    }

    void set_manual(int index) {
        _bridges->at(index)->SetUser("HSJKHSDKJHKJDHIUHHFYU&e213");
        _bridges->at(index)->SetClientKey("00000000000000000000000000000000");

        _connectionFlow->SetManual(_bridges->at(index));
    }

    void set_manual_load_retrieve_config(int index, int numGroups, int numBridges = 1, bool wasPreviousBridgeStreaming = false, int previousBridgeIndex = 0) {
        expect_message(FeedbackMessage::ID_USERPROCEDURE_STARTED, FeedbackMessage::FEEDBACK_TYPE_INFO);

        if (wasPreviousBridgeStreaming) {
            EXPECT_CALL(*_stream, Stop(_bridges->at(previousBridgeIndex))).WillOnce(Invoke(DeactivateBridge));
        }

        expect_storage_accessor_load_return_data();
        expect_small_config_retrieval_return_data(index);
        expect_initiate_full_config_retrieval();
        set_manual(index);

        _messageDispatcher->ExecutePendingActions();

        retrieve_full_config(index, numGroups, numBridges);
        _persistentData->SetActiveBridge(_bridges->at(index));
    }

    static void SetIsNotValidIp(BridgePtr bridge) {
      bridge->SetIsValidIp(false);
    }        

    BridgePtr create_bridge(int cloneIdx) {
      auto bridge = _bridges->at(cloneIdx)->Clone();
      bridge->SetUser("HSJKHSDKJHKJDHIUHHFYU&e213");
      bridge->SetClientKey("00000000000000000000000000000000");
      bridge->SetIpAddress("127.0.0.1"); // Can be anything as long as it is a valid address.
      bridge->SetModelId("BSB002"); // We need that otherwise validation will fail.
      bridge->SetIsValidIp(true);
      bridge->SetIsAuthorized(true);

      auto groups = std::make_shared<GroupList>();

      auto group1 = std::make_shared<Group>();
      group1->SetId("12");
      group1->SetName("My Entertainment Group1");
      group1->AddLight("1", 0.5, 0.4);
      group1->AddLight("2", 0.4, 0.2);
      groups->push_back(group1);

      bridge->SetGroups(groups);

      return bridge;
    }

    void connect_manual_id_and_key(int index) {
        auto bridge = create_bridge(index);

        expect_message(FeedbackMessage::ID_USERPROCEDURE_STARTED, FeedbackMessage::FEEDBACK_TYPE_INFO);
        expect_storage_accessor_load_return_data();
        expect_message(FeedbackMessage::ID_START_RETRIEVING_SMALL, FeedbackMessage::FEEDBACK_TYPE_INFO);        

        BridgeListPtr list = std::make_shared<BridgeList>();
        list->push_back(_bridges->at(index));
        auto x = std::make_shared<MockBridgeSearcher>();
        _searchers->push_back(x);        

        EXPECT_CALL(*_factory, CreateSearcher()).Times(1).WillOnce(Return(x));
        EXPECT_CALL(*x, SearchNew(_, _)).Times(1).WillOnce(DoAll(SaveArg<1>(&x->SearchNewCallback),
          InvokeArgument<1>(list)));
                        
        EXPECT_CALL(*_smallConfigRetriever, Execute(_, _, _)).Times(1).WillOnce(DoAll(
          SaveArg<0>(&_smallConfigRetriever->Bridge),
          SaveArg<1>(&_smallConfigRetriever->RetrieveCallback),
          SaveArg<2>(&_smallConfigRetriever->Feedback),
          WithArg<0>(Invoke(SetIsNotValidIp)),          
          Return(true)));

        std::string id = _bridges->at(index)->GetId();
        _connectionFlow->ConnectoToBridgeWithIdKey(id, "HSJKHSDKJHKJDHIUHHFYU&e213", "00000000000000000000000000000000");
        _messageDispatcher->ExecutePendingActions();

        // Since there will be 2 small config retrieved in a row, the first one failed and the second one succeed. We need to pause
        // the flow of events after the first retrieved otherwise we can't call EXPECT_CALL twice with different InvokeArgument<1>.

        expect_message(FeedbackMessage::ID_FINISH_RETRIEVING_FAILED, FeedbackMessage::FEEDBACK_TYPE_INFO);

        EXPECT_CALL(*_smallConfigRetriever, Execute(_, _, _)).Times(1).WillOnce(DoAll(
          SaveArg<0>(&_smallConfigRetriever->Bridge),
          SaveArg<1>(&_smallConfigRetriever->RetrieveCallback),
          SaveArg<2>(&_smallConfigRetriever->Feedback),
          InvokeArgument<1>(OPERATION_SUCCESS, _bridges->at(index)),
          Return(true)));
        EXPECT_CALL(*_fullConfigRetriever, Execute(_, _, _)).Times(1).WillOnce(DoAll(
            SaveArg<0>(&_fullConfigRetriever->Bridge),
            SaveArg<1>(&_fullConfigRetriever->RetrieveCallback),
            SaveArg<2>(&_fullConfigRetriever->Feedback),
            InvokeArgument<1>(OPERATION_SUCCESS, bridge),
            Return(true)));

        EXPECT_CALL(*_storageAccessor, Save(_, _)).Times(1).WillOnce(SaveArg<1>(&_storageAccessor->save_callback));
      
        expect_message(FeedbackMessage::ID_START_SEARCHING, FeedbackMessage::FEEDBACK_TYPE_INFO);
        expect_message(FeedbackMessage::ID_FINISH_SEARCH_BRIDGES_FOUND, FeedbackMessage::FEEDBACK_TYPE_INFO);
        expect_message(FeedbackMessage::ID_START_RETRIEVING_SMALL, FeedbackMessage::FEEDBACK_TYPE_INFO);
        expect_message(FeedbackMessage::ID_FINISH_RETRIEVING_SMALL, FeedbackMessage::FEEDBACK_TYPE_INFO);
        expect_message(FeedbackMessage::ID_START_RETRIEVING, FeedbackMessage::FEEDBACK_TYPE_INFO);
        expect_message(FeedbackMessage::ID_FINISH_RETRIEVING_READY_TO_START, FeedbackMessage::FEEDBACK_TYPE_INFO);
        expect_message(FeedbackMessage::ID_START_SAVING, FeedbackMessage::FEEDBACK_TYPE_INFO);

        // Manually call the callback since it wasn't by the first EXPECT_CALL.
        _smallConfigRetriever->ExecuteRetrieveCallback(OPERATION_FAILED, _bridges->at(index));        
        _messageDispatcher->ExecutePendingActions();       
    }

    void reset_bridge_returns_empty_bridge(bool resetAllBridges = false, int numberOfBridges = 1) {
        expect_message(FeedbackMessage::ID_USERPROCEDURE_STARTED, FeedbackMessage::FEEDBACK_TYPE_INFO);
        expect_message(FeedbackMessage::ID_DONE_RESET, FeedbackMessage::FEEDBACK_TYPE_INFO, std::make_shared<Bridge>(std::make_shared<BridgeSettings>()), BRIDGE_EMPTY);
        expect_on_storage_accessor_save(numberOfBridges, "");
        if (resetAllBridges) {
            _connectionFlow->ResetAll();
        } else {
            _connectionFlow->ResetBridge();
        }
        _messageDispatcher->ExecutePendingActions();
    }

};

INSTANTIATE_TEST_CASE_P(connect_with_manual_ip,
                        TestConnectionFlow_Manual, Values(0, 1));

TEST_P(TestConnectionFlow_Manual, connect_with_manual_ip) {
    _settings->SetAutoStartAtConnection(int2bool(GetParam()));
    connect_manual_ip_load_retrieve_push_link(0);
    finish_authorization_successfully_and_retrieve_full_config(0);
    finish(_bridges->at(0));
}

INSTANTIATE_TEST_CASE_P(connect_with_manual_ip_and_credentials,
                        TestConnectionFlow_Manual, Values(0, 1));

TEST_P(TestConnectionFlow_Manual, connect_with_manual_ip_and_credentials) {
    _settings->SetAutoStartAtConnection(int2bool(GetParam()));
    set_manual_load_retrieve_config(1, 1);
    finish(_bridges->at(1));
}

INSTANTIATE_TEST_CASE_P(connect_with_manual_id_and_key,
  TestConnectionFlow_Manual, Values(0, 1));

TEST_P(TestConnectionFlow_Manual, connect_with_manual_id_and_key) {
  _settings->SetAutoStartAtConnection(false);// int2bool(GetParam()));
  connect_manual_id_and_key(1);
  finish(_bridges->at(1));
}

INSTANTIATE_TEST_CASE_P(reset_bridge_which_was_connected_vs_not_connected,
    TestConnectionFlow_Manual, Values(0, 1));

TEST_P(TestConnectionFlow_Manual, reset_bridge_which_was_connected_vs_not_connected) {
    _settings->SetAutoStartAtConnection(false);
    set_manual_load_retrieve_config(1, 1);
    finish(_bridges->at(1));

    auto wasBridgeConnected = int2bool(GetParam());
    if (!wasBridgeConnected) {
        _bridges->at(1)->SetIsValidIp(false);
        FeedbackMessage(FeedbackMessage::REQUEST_TYPE_INTERNAL, FeedbackMessage::ID_BRIDGE_DISCONNECTED, _bridges->at(1));
    }
    reset_bridge_returns_empty_bridge();
    if (wasBridgeConnected) {
        expect_message(FeedbackMessage::ID_BRIDGE_DISCONNECTED, FeedbackMessage::FEEDBACK_TYPE_INFO);
    }
    finish_without_stream_start(false, false);
}

INSTANTIATE_TEST_CASE_P(reset_single_bridge_vs_reset_all_bridges,
    TestConnectionFlow_Manual, Values(0, 1));

TEST_P(TestConnectionFlow_Manual, reset_single_bridge_vs_reset_all_bridges) {
    _settings->SetAutoStartAtConnection(false);
    set_manual_load_retrieve_config(1, 1);
    finish(_bridges->at(1));

    set_manual_load_retrieve_config(2, 1, 2, false, 1);
    finish_without_stream_start(true, false, false);

    auto resetAllBridges = int2bool(GetParam());
    if (resetAllBridges) {
        reset_bridge_returns_empty_bridge(resetAllBridges);
    } else {
        reset_bridge_returns_empty_bridge(resetAllBridges, 2);
    }

    expect_message(FeedbackMessage::ID_BRIDGE_DISCONNECTED, FeedbackMessage::FEEDBACK_TYPE_INFO);
    finish_without_stream_start(false, false);
}

TEST_F(TestConnectionFlow_Manual, connect_with_manual_ip_followed_by_connect_with_manual_ip_and_credentials) {
    _settings->SetAutoStartAtConnection(false);
    connect_manual_ip_load_retrieve_push_link(0);

    finish_authorization_successfully_and_retrieve_full_config(0);
    finish(_bridges->at(0));

    set_manual_load_retrieve_config(1, 2, 2, false, 0);
    finish_without_stream_start(false, false, true);
}

TEST_F(TestConnectionFlow_Manual, connect_with_manual_ip_abort_and_retry) {
    connect_manual_ip_load_retrieve_push_link(0);

    expect_no_actions();
    _connectionFlow->ConnectToBridgeWithIp(_bridges->at(1)->GetIpAddress());
    _messageDispatcher->ExecutePendingActions();

    abort_finalizes();

    connect_manual_ip_load_retrieve_push_link(2);
    finish_authorization_successfully_and_retrieve_full_config(2);
    finish(_bridges->at(2));
}

TEST_F(TestConnectionFlow_Manual, connect_with_manual_ip_and_credentials_abort_and_retry) {
    expect_message(FeedbackMessage::ID_USERPROCEDURE_STARTED, FeedbackMessage::FEEDBACK_TYPE_INFO);
    expect_storage_accessor_load_return_data();
    expect_small_config_retrieval_return_data(0);
    expect_initiate_full_config_retrieval();
    set_manual(0);
    _messageDispatcher->ExecutePendingActions();

    expect_no_actions();
    set_manual(1);
    _messageDispatcher->ExecutePendingActions();

    abort_finalizes();

    set_manual_load_retrieve_config(2, 1, 2, false, 1);
    finish(_bridges->at(2));
}

TEST_P(TestConnectionFlow_Manual, add_manual_already_known_bridge_doesnt_duplicate_entry) {
    _settings->SetAutoStartAtConnection(false);

    set_manual_load_retrieve_config(0, 1);
    finish_without_stream_start(true);

    set_manual_load_retrieve_config(1, 1, 2, false, 0);
    finish_without_stream_start(true, false, false);

    set_manual_load_retrieve_config(0, 1, 2, false, 1);
    finish_without_stream_start(true, false, false);

    set_manual_load_retrieve_config(2, 1, 3, false, 0);
    finish_without_stream_start(true, false, false);

    set_manual_load_retrieve_config(1, 1, 3, false, 2);
    finish_without_stream_start(true, false, false);

    set_manual_load_retrieve_config(0, 1, 3, false, 1);
    finish_without_stream_start(true, false, false);
}

TEST_P(TestConnectionFlow_Manual, add_manual_when_already_streaming_first_stops_streaming) {
    _settings->SetAutoStartAtConnection(true);

    set_manual_load_retrieve_config(0, 1);
    finish(_bridges->at(0));
    ASSERT_EQ(_bridges->at(0)->GetStatus(), BRIDGE_STREAMING);

    set_manual_load_retrieve_config(1, 1, 2, true, 0);
    finish_with_stream_start(_bridges->at(1), false, false, false);
}

TEST_F(TestConnectionFlow_Manual, api_version_empty__expect_small_and_full_config_retrieval) {
    set_manual_load_retrieve_config(3, 1);
}

TEST_F(TestConnectionFlow_Manual, ManualBridge__SmallConfigHasOldApi) {
    expect_message(FeedbackMessage::ID_USERPROCEDURE_STARTED, FeedbackMessage::FEEDBACK_TYPE_INFO);
    expect_storage_accessor_load_return_data();
    // small config returns bridge with old api
    expect_small_config_retrieval_return_data(1);

    set_manual(4);
    expect_initiate_full_config_retrieval();

    _messageDispatcher->ExecutePendingActions();
    EXPECT_EQ(_persistentData->GetActiveBridge()->GetApiversion(), "1.24.0");
}

INSTANTIATE_TEST_CASE_P(manual_bridge_https, TestConnectionFlow_Manual, Values(true, false));

TEST_P(TestConnectionFlow_Manual, ManualBridgeHttps__NoExistingBridges__SmallConfigHasNewApi) {
    bool connect_by_ip = GetParam();

    expect_message(FeedbackMessage::ID_USERPROCEDURE_STARTED, FeedbackMessage::FEEDBACK_TYPE_INFO);
    expect_storage_accessor_load_return_data();
    // small config returns bridge with new api
    expect_small_config_retrieval_return_data(4);

    if (connect_by_ip) {
        expect_message(FeedbackMessage::ID_START_AUTHORIZING, FeedbackMessage::FEEDBACK_TYPE_INFO);
        expect_message(FeedbackMessage::ID_PRESS_PUSH_LINK, FeedbackMessage::FEEDBACK_TYPE_USER);

        _connectionFlow->ConnectToBridgeWithIp(_bridges->at(4)->GetIpAddress());
        expect_on_authenticator_authenticate(4);
    }
    else {
        set_manual(4);
        expect_initiate_full_config_retrieval();
    }

    _messageDispatcher->ExecutePendingActions();
}

TEST_P(TestConnectionFlow_Manual, ManualBridgeHttps__ExistingBridge__SmallConfigHasNewApi) {
    bool connect_by_ip = GetParam();

    expect_message(FeedbackMessage::ID_USERPROCEDURE_STARTED, FeedbackMessage::FEEDBACK_TYPE_INFO);
    expect_storage_accessor_load_return_data();
    // small config returns bridge with new api
    expect_small_config_retrieval_return_data(4);

    _persistentData->SetActiveBridge(_bridges->at(4)->Clone());
    _persistentData->GetActiveBridge()->SetUser("HSJKHSDKJHKJDHIUHHFYU&e213");
    _persistentData->GetActiveBridge()->SetClientKey("00000000000000000000000000000000");

    if (connect_by_ip) {
        expect_message(FeedbackMessage::ID_START_AUTHORIZING, FeedbackMessage::FEEDBACK_TYPE_INFO);
        expect_message(FeedbackMessage::ID_PRESS_PUSH_LINK, FeedbackMessage::FEEDBACK_TYPE_USER);

        _connectionFlow->ConnectToBridgeWithIp(_bridges->at(4)->GetIpAddress());
        expect_on_authenticator_authenticate(4);
    }
    else {
        set_manual(4);
        expect_initiate_full_config_retrieval();
    }

    _messageDispatcher->ExecutePendingActions();
}

TEST_P(TestConnectionFlow_Manual, ManualBridgeHttps__ExistingBridge__SmallConfigHasOldApi) {
    bool connect_by_ip = GetParam();

    auto bridge_with_old_api = std::make_shared<Bridge>(std::make_shared<BridgeSettings>());
    bridge_with_old_api->SetModelId("BSB002");
    bridge_with_old_api->SetApiversion("1.24.0");
    bridge_with_old_api->SetSwversion("1940094000");
    bridge_with_old_api->SetId(_bridges->at(4)->GetId());

    _persistentData->SetActiveBridge(_bridges->at(4)->Clone());
    _persistentData->GetActiveBridge()->SetUser("HSJKHSDKJHKJDHIUHHFYU&e213");
    _persistentData->GetActiveBridge()->SetClientKey("00000000000000000000000000000000");

    expect_message(FeedbackMessage::ID_USERPROCEDURE_STARTED, FeedbackMessage::FEEDBACK_TYPE_INFO);
    expect_storage_accessor_load_return_data();
    // small config returns bridge with old api
    expect_small_config_retrieval_return_data(bridge_with_old_api);

    if (connect_by_ip) {
        expect_message(FeedbackMessage::ID_START_AUTHORIZING, FeedbackMessage::FEEDBACK_TYPE_INFO);
        expect_message(FeedbackMessage::ID_PRESS_PUSH_LINK, FeedbackMessage::FEEDBACK_TYPE_USER);

        _connectionFlow->ConnectToBridgeWithIp(_persistentData->GetActiveBridge()->GetIpAddress());
        expect_on_authenticator_authenticate(4);
    }
    else {
        set_manual(4);
        expect_initiate_full_config_retrieval();
    }

    _messageDispatcher->ExecutePendingActions();
    EXPECT_EQ(_persistentData->GetActiveBridge()->GetApiversion(), "1.24.0");
}

TEST_P(TestConnectionFlow_Manual, ManualBridgeHttps__NoExistingBridges__OldModelId__SmallConfigHasNewApi__BridgeSearchStarts) {
    bool connect_by_ip = GetParam();

    expect_message(FeedbackMessage::ID_USERPROCEDURE_STARTED, FeedbackMessage::FEEDBACK_TYPE_INFO);
    expect_storage_accessor_load_return_data();
    // small config returns bridge with new api but old model
    expect_small_config_retrieval_return_data(5);
    expect_message(FeedbackMessage::ID_START_SEARCHING, FeedbackMessage::FEEDBACK_TYPE_INFO);
    expect_on_searcher_search_new(false);

    EXPECT_FALSE(_bridges->at(5)->IsValidModelId());
    if (connect_by_ip) {
        _connectionFlow->ConnectToBridgeWithIp(_bridges->at(5)->GetIpAddress());
    }
    else {
        set_manual(5);
    }

    _messageDispatcher->ExecutePendingActions();
}