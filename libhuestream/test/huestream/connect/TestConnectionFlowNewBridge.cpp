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
#include "huestream/config/Config.h"

using namespace testing;
using namespace huestream;

constexpr static auto PUSHLINK_INVALID_IP_MAX_RETRIES = 3;

class TestConnectionFlow_NewBridge : public TestConnectionFlowBase {
public:

    void connect_with_no_bridge_configured_starts_new_bridge_search(bool background = false) {
        connect_starts_bridge_loading(background);

        expect_on_searcher_search_new(false);
        expect_message(FeedbackMessage::ID_FINISH_LOADING_NO_BRIDGE_CONFIGURED, FeedbackMessage::FEEDBACK_TYPE_INFO, std::make_shared<Bridge>(std::make_shared<BridgeSettings>()));
        expect_message(FeedbackMessage::ID_START_SEARCHING, FeedbackMessage::FEEDBACK_TYPE_INFO);
        _storageAccessor->load_callback(OPERATION_FAILED, std::make_shared<HueStreamData>(std::make_shared<BridgeSettings>()));
        _messageDispatcher->ExecutePendingActions();
    }

    void connect_with_background_result_starts_pushlink(int index) {
        expect_message(FeedbackMessage::ID_USERPROCEDURE_STARTED, FeedbackMessage::FEEDBACK_TYPE_INFO);
        expect_message(FeedbackMessage::ID_START_AUTHORIZING, FeedbackMessage::FEEDBACK_TYPE_INFO);
        expect_message(FeedbackMessage::ID_PRESS_PUSH_LINK, FeedbackMessage::FEEDBACK_TYPE_USER);
        expect_on_authenticator_authenticate(index);

        _connectionFlow->ConnectToBridge();
        _messageDispatcher->ExecutePendingActions();
    }

    void connect_background_with_no_bridge_configured_starts_new_bridge_search() {
        expect_message(FeedbackMessage::ID_USERPROCEDURE_STARTED, FeedbackMessage::FEEDBACK_TYPE_INFO);
        expect_message(FeedbackMessage::ID_START_LOADING, FeedbackMessage::FEEDBACK_TYPE_INFO);
        expect_on_storage_accessor_load();
        _connectionFlow->ConnectToBridge();
        _messageDispatcher->ExecutePendingActions();

        expect_on_searcher_search_new(false);
        expect_message(FeedbackMessage::ID_FINISH_LOADING_NO_BRIDGE_CONFIGURED, FeedbackMessage::FEEDBACK_TYPE_INFO, std::make_shared<Bridge>(std::make_shared<BridgeSettings>()));
        expect_message(FeedbackMessage::ID_START_SEARCHING, FeedbackMessage::FEEDBACK_TYPE_INFO);
        _storageAccessor->load_callback(OPERATION_FAILED, std::make_shared<HueStreamData>(std::make_shared<BridgeSettings>()));
        _messageDispatcher->ExecutePendingActions();
    }

    void searching_with_one_bridge_found_but_is_ignored() {
        auto searcher = get_last_searcher();

        expect_message(FeedbackMessage::ID_FINISH_SEARCHING_NO_BRIDGES_FOUND, FeedbackMessage::FEEDBACK_TYPE_INFO);
        expect_message(FeedbackMessage::ID_START_SEARCHING, FeedbackMessage::FEEDBACK_TYPE_INFO);
        expect_on_searcher_search_new(true);
        finish_search_with_one_bridge(searcher, 0);
    }

    void finish_authorization_unsuccessfully_start_new_authentication(int index) {
        auto authenticator = _authenticators->at(index);
        expect_on_authenticator_authenticate(index);
        authenticator->authenticate_callback(_bridges->at(index));
        // Process authorization completed event
        _messageDispatcher->ExecutePendingActions();
        // Execute delayed authorization retry attempt
        _scheduler->execute_scheduled_callback();
        _messageDispatcher->ExecutePendingActions();
    }

    void authenticate_connection_failure_once(int index) {
        auto callback = _authenticators->at(index);

        // Invalidate bridge and trigger authorization complete event
        _bridges->at(index)->SetIsValidIp(false);
        callback->authenticate_callback(_bridges->at(index));
        _messageDispatcher->ExecutePendingActions();

        // Execute delayed authorization retry attempt
        _scheduler->execute_scheduled_callback();
        _messageDispatcher->ExecutePendingActions();

        _scheduler->add_task({});
    }

    void authenticate_connection_failure_with_retries(int index) {
        auto callback = _authenticators->at(index);

        for (int i = 0; i < PUSHLINK_INVALID_IP_MAX_RETRIES + 1; ++i) {
            // Execute delayed authorization retry attempt
            _scheduler->execute_scheduled_callback();
            _messageDispatcher->ExecutePendingActions();

            // Invalidate bridge and trigger authorization complete event
            _bridges->at(index)->SetIsValidIp(false);
            callback->authenticate_callback(_bridges->at(index));
            _messageDispatcher->ExecutePendingActions();
        }

        _scheduler->add_task({});
    }

    void trigger_authorization_failure(int index) {
        expect_on_authenticator_authenticate(index);

        authenticate_connection_failure_once(index);
    }

    void finish_authorization_with_connection_failure_ignores_bridge(int index) {
        expect_on_authenticator_authenticate(index, PUSHLINK_INVALID_IP_MAX_RETRIES);

        authenticate_connection_failure_with_retries(index);
    }

    void finish_authorization_with_connection_failure_ignores_bridge_and_starts_new_search(int index) {
        expect_on_authenticator_authenticate(index, PUSHLINK_INVALID_IP_MAX_RETRIES);

        auto callback = _authenticators->at(index);
        expect_on_searcher_search_new(false);
        expect_message(FeedbackMessage::ID_FINISH_AUTHORIZING_FAILED, FeedbackMessage::FEEDBACK_TYPE_INFO);
        expect_message(FeedbackMessage::ID_START_SEARCHING, FeedbackMessage::FEEDBACK_TYPE_INFO);
        authenticate_connection_failure_with_retries(index);
    }

    void finish_authorization_when_another_bridge_already_authorized_ignores_bridge(int index) {
        authorize_bridge(index);
        expect_no_actions();
        _authenticators->at(index)->authenticate_callback(_bridges->at(index));
        _messageDispatcher->ExecutePendingActions();
    }

    void connect_to_bridge_successfully(int index, int numGroups = 1) {
        connect_with_no_bridge_configured_starts_new_bridge_search();
        searching_with_one_bridge_found_starts_pushlink(0);
        finish_authorization_successfully_and_retrieve_full_config(index, numGroups);
        _persistentData->SetActiveBridge(_bridges->at(index));
        if (numGroups == 0) {
            finish_without_stream_start(false, true, false);
        } else if (numGroups == 1) {
            finish(_bridges->at(index));
        } else {
            finish_without_stream_start(false);
        }
    }

    void start_stream_activation() {
        expect_message(FeedbackMessage::ID_USERPROCEDURE_STARTED, FeedbackMessage::FEEDBACK_TYPE_INFO);
        expect_initiate_full_config_retrieval();
        _connectionFlow->StartStream(nullptr);
        _messageDispatcher->ExecutePendingActions();

        expect_message(FeedbackMessage::ID_FINISH_RETRIEVING_READY_TO_START, FeedbackMessage::FEEDBACK_TYPE_INFO, _bridges->at(0), BRIDGE_READY);
        expect_on_storage_accessor_save();
        _fullConfigRetriever->RetrieveCallback(OPERATION_SUCCESS, _fullConfigRetriever->Bridge);
        _messageDispatcher->ExecutePendingActions();
    }

    void setup_for_stream_activation() {
        _settings->SetAutoStartAtConnection(false);
        connect_to_bridge_successfully(0, 1);

        start_stream_activation();
    }

    void connect_new_bridge_starts_searching(int previousBridgeIndex, bool wasPreviouslyStreaming = false) {
        expect_message(FeedbackMessage::ID_USERPROCEDURE_STARTED, FeedbackMessage::FEEDBACK_TYPE_INFO);

        expect_storage_accessor_load_return_data();

        if (wasPreviouslyStreaming) {
            EXPECT_CALL(*_stream, Stop(_bridges->at(previousBridgeIndex))).WillOnce(Invoke(DeactivateBridge));
        }
        expect_message(FeedbackMessage::ID_START_SEARCHING, FeedbackMessage::FEEDBACK_TYPE_INFO);
        expect_on_searcher_search_new(false);

        _connectionFlow->ConnectToNewBridge();
        _messageDispatcher->ExecutePendingActions();
    }
};

TEST_F(TestConnectionFlow_NewBridge, searching_can_be_aborted) {
    connect_with_no_bridge_configured_starts_new_bridge_search();
    searching_once_with_no_bridges_found_starts_new_bridge_search();
    searching_with_abort_finalizes();
}

INSTANTIATE_TEST_CASE_P(search_twice_and_authenticate_with_multiple_bridges_found,
                        TestConnectionFlow_NewBridge, Values(0, 1, 2));

TEST_P(TestConnectionFlow_NewBridge, search_twice_and_authenticate_with_multiple_bridges_found) {

    connect_with_no_bridge_configured_starts_new_bridge_search();
    searching_once_with_no_bridges_found_starts_new_bridge_search();
    searching_with_three_bridges_found_starts_pushlink_on_all_three_bridges();
    finish_authorization_successfully_and_retrieve_full_config(GetParam());
    finish(_bridges->at(GetParam()));
}

INSTANTIATE_TEST_CASE_P(search_once_and_authenticate_single_new_bridge,
                        TestConnectionFlow_NewBridge, Values(0, 1, 2));

TEST_P(TestConnectionFlow_NewBridge, search_once_and_authenticate_single_new_bridge) {
    connect_to_bridge_successfully(0, GetParam());
}

INSTANTIATE_TEST_CASE_P(connect_succesfully_with_or_without_stream_start,
                        TestConnectionFlow_NewBridge, Values(0, 1));

TEST_P(TestConnectionFlow_NewBridge, connect_succesfully_with_or_without_stream_start) {
    _settings->SetAutoStartAtConnection(int2bool(GetParam()));
    connect_to_bridge_successfully(0, 1);
}

TEST_F(TestConnectionFlow_NewBridge, ignores_bridge_from_searcher_with_invalid_ip) {
    connect_with_no_bridge_configured_starts_new_bridge_search();
    _bridges->at(0)->SetIsValidIp(false);
    searching_with_one_bridge_found_but_is_ignored();
}

TEST_F(TestConnectionFlow_NewBridge, search_twice_finish_with_no_bridges_found) {
    connect_with_no_bridge_configured_starts_new_bridge_search();
    searching_once_with_no_bridges_found_starts_new_bridge_search();
    searching_twice_with_no_bridges_found_finalizes();
}

TEST_F(TestConnectionFlow_NewBridge, authentication_repeats_until_authenticated) {
    connect_with_no_bridge_configured_starts_new_bridge_search();
    searching_with_one_bridge_found_starts_pushlink(0);

    finish_authorization_unsuccessfully_start_new_authentication(0);
    finish_authorization_unsuccessfully_start_new_authentication(0);
    finish_authorization_unsuccessfully_start_new_authentication(0);
    finish_authorization_successfully_and_retrieve_full_config(0);
}

TEST_F(TestConnectionFlow_NewBridge, authentication_repeats_until_abort) {
    connect_with_no_bridge_configured_starts_new_bridge_search();
    searching_with_one_bridge_found_starts_pushlink(0);
    finish_authorization_unsuccessfully_start_new_authentication(0);
    abort_finalizes();
}

TEST_F(TestConnectionFlow_NewBridge, authentication_repeats_until_abort_multiple_bridges) {
    connect_with_no_bridge_configured_starts_new_bridge_search();
    searching_with_three_bridges_found_starts_pushlink_on_all_three_bridges();
    finish_authorization_unsuccessfully_start_new_authentication(0);
    finish_authorization_unsuccessfully_start_new_authentication(1);
    finish_authorization_unsuccessfully_start_new_authentication(2);
    abort_finalizes();
}

TEST_F(TestConnectionFlow_NewBridge, authentication_connection_failure_triggers_new_search_after_several_attempts) {
    connect_with_no_bridge_configured_starts_new_bridge_search();
    searching_with_one_bridge_found_starts_pushlink(0);
    finish_authorization_with_connection_failure_ignores_bridge_and_starts_new_search(0);
}

TEST_F(TestConnectionFlow_NewBridge, authentication_connection_failure_multiple_bridges_triggers_new_search_after_several_attempts) {
    connect_with_no_bridge_configured_starts_new_bridge_search();
    searching_with_three_bridges_found_starts_pushlink_on_all_three_bridges();
    finish_authorization_with_connection_failure_ignores_bridge(0);
    finish_authorization_with_connection_failure_ignores_bridge(1);
    finish_authorization_with_connection_failure_ignores_bridge_and_starts_new_search(2);
}

TEST_F(TestConnectionFlow_NewBridge, authentication_connection_failure_and_success) {
    connect_with_no_bridge_configured_starts_new_bridge_search();
    searching_with_three_bridges_found_starts_pushlink_on_all_three_bridges();
    finish_authorization_with_connection_failure_ignores_bridge(0);
    finish_authorization_unsuccessfully_start_new_authentication(1);
    finish_authorization_successfully_and_retrieve_full_config(2);
}

TEST_F(TestConnectionFlow_NewBridge, multiple_authentications_only_returns_one_bridge) {
    connect_with_no_bridge_configured_starts_new_bridge_search();
    searching_with_three_bridges_found_starts_pushlink_on_all_three_bridges();
    finish_authorization_successfully(0);
    finish_authorization_when_another_bridge_already_authorized_ignores_bridge(1);
    retrieve_full_config(0, 1);
}

TEST_F(TestConnectionFlow_NewBridge, connect_twice_while_searching_is_ignored_but_ok_after_abort) {
    connect_with_no_bridge_configured_starts_new_bridge_search();
    expect_no_actions();
    _connectionFlow->ConnectToBridge();
    _messageDispatcher->ExecutePendingActions();

    searching_with_abort_finalizes();
    connect_with_no_bridge_configured_starts_new_bridge_search();
    _connectionFlow->ConnectToBridge();
}

TEST_F(TestConnectionFlow_NewBridge, connect_twice_while_autenticating_is_ignored) {
    connect_with_no_bridge_configured_starts_new_bridge_search();
    searching_with_one_bridge_found_starts_pushlink(0);
    expect_no_actions();
    _connectionFlow->ConnectToBridge();
    _messageDispatcher->ExecutePendingActions();
}

TEST_F(TestConnectionFlow_NewBridge, select_existing_group_saves_bridge) {
    connect_to_bridge_successfully(0, 2);

    select_group_existing(0, "12");
    finish_with_stream_start(_bridges->at(0), false, true, true, false);
}

TEST_F(TestConnectionFlow_NewBridge, select_not_existing_group_saves_bridge) {
    connect_to_bridge_successfully(0);

    expect_message(FeedbackMessage::ID_USERPROCEDURE_STARTED, FeedbackMessage::FEEDBACK_TYPE_INFO);
    expect_message(FeedbackMessage::ID_USERPROCEDURE_FINISHED, FeedbackMessage::FEEDBACK_TYPE_INFO);
    _connectionFlow->SelectGroup("1");
    _messageDispatcher->ExecutePendingActions();
}

TEST_F(TestConnectionFlow_NewBridge, change_group_while_streaming) {
    connect_to_bridge_successfully(0, 2);
    ASSERT_FALSE(_bridges->at(0)->IsStreaming());

    select_group_existing(0, "12");
    finish_with_stream_start(_bridges->at(0), false, true, true, false);
    ASSERT_TRUE(_bridges->at(0)->IsStreaming());

    select_group_existing(0, "13");
    finish_with_stream_start(_bridges->at(0), false, true, false, false);
    ASSERT_TRUE(_bridges->at(0)->IsStreaming());
}

TEST_F(TestConnectionFlow_NewBridge, change_group_while_streaming_no_autostart) {
    _settings->SetAutoStartAtConnection(false);
    connect_to_bridge_successfully(0, 2);
    ASSERT_FALSE(_bridges->at(0)->IsStreaming());

    select_group_existing(0, "12");
    finish_without_stream_start(true, false, false, false);
    ASSERT_FALSE(_bridges->at(0)->IsStreaming());

    start_stream_activation();
    finish_with_stream_start(_bridges->at(0), false, true, true, false);
    ASSERT_TRUE(_bridges->at(0)->IsStreaming());

    select_group_existing(0, "13");
    finish_with_stream_start(_bridges->at(0), false, true, false, false);
    ASSERT_TRUE(_bridges->at(0)->IsStreaming());
}

INSTANTIATE_TEST_CASE_P(start_successful, TestConnectionFlow_NewBridge, Values(0, 1));

TEST_P(TestConnectionFlow_NewBridge, start_successful) {
    _settings->SetUseRenderThread(int2bool(GetParam()));

    setup_for_stream_activation();

    finish_with_stream_start(_bridges->at(0), false, true, true, false);
}

TEST_F(TestConnectionFlow_NewBridge, start_fail) {
    setup_for_stream_activation();

    expect_message(FeedbackMessage::ID_FINISH_SAVING_SAVED, FeedbackMessage::FEEDBACK_TYPE_INFO);
    expect_message(FeedbackMessage::ID_START_ACTIVATING, FeedbackMessage::FEEDBACK_TYPE_INFO);
    expect_message(FeedbackMessage::ID_FINISH_ACTIVATING_FAILED, FeedbackMessage::FEEDBACK_TYPE_INFO, _bridges->at(0), BRIDGE_BUSY);
    expect_message(FeedbackMessage::ID_DONE_ACTION_REQUIRED, FeedbackMessage::FEEDBACK_TYPE_INFO, _bridges->at(0), BRIDGE_BUSY);
    expect_message(FeedbackMessage::ID_BUSY_STREAMING, FeedbackMessage::FEEDBACK_TYPE_USER);
    expect_message(FeedbackMessage::ID_USERPROCEDURE_FINISHED, FeedbackMessage::FEEDBACK_TYPE_INFO);
    EXPECT_CALL(*_stream, StartWithRenderThread(_)).WillOnce(Invoke(FailActivateBridgeBusy));

    _storageAccessor->save_callback(OPERATION_SUCCESS);
    _messageDispatcher->ExecutePendingActions();
}

TEST_F(TestConnectionFlow_NewBridge, connect_background_bridge_found) {
    connect_with_no_bridge_configured_starts_new_bridge_search(true);
    searching_background_with_one_bridge_found_finishes(0);

    connect_with_background_result_starts_pushlink(0);
    finish_authorization_successfully_and_retrieve_full_config(0, 1);
    finish(_bridges->at(0));
}

TEST_F(TestConnectionFlow_NewBridge, stop) {
    _settings->SetAutoStartAtConnection(true);
    connect_to_bridge_successfully(0, 1);
    ASSERT_EQ(_bridges->at(0)->GetStatus(), BRIDGE_STREAMING);

    expect_message(FeedbackMessage::ID_USERPROCEDURE_STARTED, FeedbackMessage::FEEDBACK_TYPE_INFO);
    expect_message(FeedbackMessage::ID_STREAMING_DISCONNECTED, FeedbackMessage::FEEDBACK_TYPE_USER, _bridges->at(0), BRIDGE_READY);
    expect_message(FeedbackMessage::ID_GROUPLIST_UPDATED, FeedbackMessage::FEEDBACK_TYPE_INFO);
    expect_message(FeedbackMessage::ID_USERPROCEDURE_FINISHED, FeedbackMessage::FEEDBACK_TYPE_INFO);

    EXPECT_CALL(*_stream, Stop(_bridges->at(0))).WillOnce(Invoke(DeactivateBridge));

    _connectionFlow->StopStream(nullptr);
    _messageDispatcher->ExecutePendingActions();
}

TEST_F(TestConnectionFlow_NewBridge, connect_background_no_bridge_found) {
    connect_with_no_bridge_configured_starts_new_bridge_search(true);
    searching_background_with_no_bridge_found_finishes();
}

TEST_F(TestConnectionFlow_NewBridge, connect_background_v1_bridge_ignored) {
    connect_with_no_bridge_configured_starts_new_bridge_search(true);
    searching_background_with_only_v1_bridge_found_finishes();
}

TEST_F(TestConnectionFlow_NewBridge, OnMonitorEventDisconnect) {
    connect_to_bridge_successfully(0, 1);

    _bridges->at(0)->GetGroup()->SetActive(false);
    expect_message(FeedbackMessage::ID_STREAMING_DISCONNECTED, FeedbackMessage::FEEDBACK_TYPE_USER, _bridges->at(0), BRIDGE_READY);

    EXPECT_CALL(*_stream, Stop(_bridges->at(0))).WillOnce(Invoke(DeactivateBridge));

    _connectionFlow->OnBridgeMonitorEvent(FeedbackMessage(FeedbackMessage::REQUEST_TYPE_INTERNAL, FeedbackMessage::ID_STREAMING_DISCONNECTED, _bridges->at(0)));
    _messageDispatcher->ExecutePendingActions();
}

TEST_F(TestConnectionFlow_NewBridge, OnMonitorEventLightsChanged) {
    connect_to_bridge_successfully(0, 1);

    expect_message(FeedbackMessage::ID_LIGHTS_UPDATED, FeedbackMessage::FEEDBACK_TYPE_INFO, _bridges->at(0));

    _connectionFlow->OnBridgeMonitorEvent(FeedbackMessage(FeedbackMessage::REQUEST_TYPE_INTERNAL, FeedbackMessage::ID_LIGHTS_UPDATED, _bridges->at(0)));
    _messageDispatcher->ExecutePendingActions();
}

TEST_F(TestConnectionFlow_NewBridge, connect_to_new_bridge) {
    _settings->SetAutoStartAtConnection(false);
    connect_to_bridge_successfully(0, 2);

    connect_new_bridge_starts_searching(0);

    searching_with_three_bridges_found_starts_pushlink_on_all_three_bridges();
    finish_authorization_successfully_and_retrieve_full_config(1, 1, 2);
    finish_without_stream_start(true, false);
}

TEST_F(TestConnectionFlow_NewBridge, connect_to_new_bridge_stops_streaming_bridge) {
    _settings->SetAutoStartAtConnection(true);
    connect_to_bridge_successfully(0, 1);
    ASSERT_EQ(_bridges->at(0)->GetStatus(), BRIDGE_STREAMING);

    connect_new_bridge_starts_searching(0, true);

    searching_with_three_bridges_found_starts_pushlink_on_all_three_bridges();
    finish_authorization_successfully_and_retrieve_full_config(1, 2, 2);
    expect_message(FeedbackMessage::ID_STREAMING_DISCONNECTED, FeedbackMessage::FEEDBACK_TYPE_USER);
    finish_without_stream_start(false, false);
}

TEST_P(TestConnectionFlow_NewBridge, connect_stores_bridgelist) {
    connect_to_bridge_successfully(0);
}

TEST_F(TestConnectionFlow_NewBridge, connect_to_new_bridge_succeedes_after_several_failed_attempts) {
    connect_with_no_bridge_configured_starts_new_bridge_search(true);
    searching_background_with_one_bridge_found_finishes(0);
    connect_with_background_result_starts_pushlink(0);

    trigger_authorization_failure(0);
    trigger_authorization_failure(0);
    trigger_authorization_failure(0);
    finish_authorization_successfully_and_retrieve_full_config(0, 1);

    finish(_bridges->at(0));
}

TEST_F(TestConnectionFlow_NewBridge, connect_to_new_bridge_fails_after_several_failed_attempts) {
    connect_with_no_bridge_configured_starts_new_bridge_search(true);
    searching_background_with_one_bridge_found_finishes(0);
    connect_with_background_result_starts_pushlink(0);

    trigger_authorization_failure(0);
    trigger_authorization_failure(0);
    trigger_authorization_failure(0);

    // Next authorization will fail and start new search
    expect_message(FeedbackMessage::ID_FINISH_AUTHORIZING_FAILED, FeedbackMessage::FEEDBACK_TYPE_INFO);
    expect_message(FeedbackMessage::ID_START_SEARCHING, FeedbackMessage::FEEDBACK_TYPE_INFO);
    EXPECT_CALL(*_factory, CreateSearcher()).Times(1).WillOnce(Return(std::make_shared<NiceMock<MockBridgeSearcher>>()));
    authenticate_connection_failure_once(0);
    abort_finalizes();
}

INSTANTIATE_TEST_CASE_P(new_bridge_https, TestConnectionFlow_NewBridge, Values(true, false));

TEST_P(TestConnectionFlow_NewBridge, new_bridge_https) {
    connect_with_no_bridge_configured_starts_new_bridge_search(GetParam());

    if (GetParam()) {
        searching_background_with_one_bridge_found_finishes(4);
    }
    else {
        searching_with_one_bridge_found_starts_pushlink(4);
    }
}
