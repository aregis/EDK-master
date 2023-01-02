#include "gtest/gtest.h"
#include "TestConnectionFlowBase.h"
#include "test/huestream/_mock/MockConfigRetriever.h"

#include <huestream/config/Config.h>
#include <huestream/HueStream.h>

using namespace testing;
using namespace huestream;

using std::string;

void TestConnectionFlowBase::SetUp() {
    _config = std::make_shared<Config>("name", "platform", PersistenceEncryptionKey{"encryption_key"});
    _settings = std::make_shared<AppSettings>();
    _settings->SetUseRenderThread(true);
    _config->SetAppSettings(_settings);
    _stream = std::make_shared<MockStream>();
    _persistentData = std::make_shared<HueStreamData>(std::make_shared<BridgeSettings>());

    _bridges = std::make_shared<BridgeList>(6);
    _bridges->at(0) = std::make_shared<Bridge>("001788FFFE2007AA", "192.168.1.34", true, std::make_shared<BridgeSettings>());
    _bridges->at(1) = std::make_shared<Bridge>("001788FFFE2007BB", "192.168.1.35", true, std::make_shared<BridgeSettings>());
    _bridges->at(2) = std::make_shared<Bridge>("001788FFFE2007CC", "192.168.1.36", true, std::make_shared<BridgeSettings>());
    _bridges->at(3) = std::make_shared<Bridge>("001788FFFE2007DD", "192.168.1.36", true, std::make_shared<BridgeSettings>());
    _bridges->at(4) = std::make_shared<Bridge>("001788FFFE2007EE", "192.168.1.36", true, std::make_shared<BridgeSettings>());
    _bridges->at(5) = std::make_shared<Bridge>("001788FFFE2007FF", "192.168.1.36", true, std::make_shared<BridgeSettings>());
    _bridges->at(0)->SetApiversion("1.24.0");
    _bridges->at(1)->SetApiversion("1.24.0");
    _bridges->at(2)->SetApiversion("1.24.0");
    _bridges->at(3)->SetApiversion("");
    _bridges->at(4)->SetApiversion("1.24.0");
    _bridges->at(5)->SetApiversion("1.24.0");
    _bridges->at(0)->SetModelId("BSB002");
    _bridges->at(1)->SetModelId("BSB002");
    _bridges->at(2)->SetModelId("BSB002");
    _bridges->at(3)->SetModelId("BSB002");
    _bridges->at(4)->SetModelId("BSB002");
    _bridges->at(5)->SetModelId("BSB001");
    _bridges->at(0)->SetSwversion("1940094000");
    _bridges->at(1)->SetSwversion("1940094000");
    _bridges->at(2)->SetSwversion("1940094000");
    _bridges->at(3)->SetSwversion("1940094000");
    _bridges->at(4)->SetSwversion("1940094000");
    _bridges->at(5)->SetSwversion("1940094000");

    _searchers = std::make_shared<vector<shared_ptr<MockBridgeSearcher>>>();
    _authenticators = std::make_shared<vector<shared_ptr<MockBridgeAuthenticator>>>(6);
    _factory = std::make_shared<MockConnectionFlowFactory>();
    _scheduler = std::make_shared<NiceMock<MockScheduler>>();
    support::SchedulerFactory::set_factory_method([this](unsigned int interval) {
        return std::unique_ptr<MockWrapperScheduler>(new MockWrapperScheduler(interval, _scheduler));
    });
    _messageDispatcher = std::make_shared<StubMessageDispatcher>();
    EXPECT_CALL(*_factory, GetMessageDispatcher()).WillRepeatedly(Return(_messageDispatcher));
    _storageAccessor = std::make_shared<MockBridgeStorageAccessor>();
    _smallConfigRetriever = std::make_shared<MockConfigRetriever>();
    _fullConfigRetriever = std::make_shared<MockConfigRetriever>();
    EXPECT_CALL(*_factory, CreateConfigRetriever(_, ConfigType::Small, false)).WillRepeatedly(Return(_smallConfigRetriever));
    EXPECT_CALL(*_factory, CreateConfigRetriever(_, ConfigType::Full, false)).WillRepeatedly(Return(_fullConfigRetriever));
    _connectionFlow = std::make_shared<TestableConnectionFlow>(_factory, _stream, std::make_shared<BridgeSettings>(), _settings, _storageAccessor);
    ON_CALL(*_connectionFlow, NewMessage(_)).WillByDefault(Invoke(PrintDebugMsg));

    //No need to verify ID_LIGHTS_UPDATED and ID_GROUP_LIGHTSTATE_UPDATED messages in connectionflow test
    EXPECT_CALL(*_connectionFlow, NewMessage(FbMessage(FeedbackMessage::ID_LIGHTS_UPDATED, FeedbackMessage::FEEDBACK_TYPE_INFO))).Times(AtLeast(0));
    EXPECT_CALL(*_connectionFlow, NewMessage(FbMessage(FeedbackMessage::ID_GROUP_LIGHTSTATE_UPDATED, FeedbackMessage::FEEDBACK_TYPE_INFO))).Times(AtLeast(0));
}

void TestConnectionFlowBase::TearDown() {
}

bool TestConnectionFlowBase::int2bool(int param) {
    if (param == 0)
        return false;
    return true;
}

void TestConnectionFlowBase::expect_message(FeedbackMessage::Id id, FeedbackMessage::FeedbackType type) {
    PrintExpectMsg(id);
    EXPECT_CALL(*_connectionFlow, NewMessage(FbMessage(id, type))).Times(1).WillRepeatedly(Invoke(PrintDebugMsg));
}

void TestConnectionFlowBase::expect_message(FeedbackMessage::Id id, FeedbackMessage::FeedbackType type, BridgePtr bridge) {
    PrintExpectMsg(id);
    EXPECT_CALL(*_connectionFlow, NewMessage(FbMessageWithBridge(id, type, bridge))).Times(1).WillRepeatedly(Invoke(PrintDebugMsg));
}

void TestConnectionFlowBase::expect_message(FeedbackMessage::Id id, FeedbackMessage::FeedbackType type, BridgePtr bridge, BridgeStatus status) {
    PrintExpectMsg(id);
    EXPECT_CALL(*_connectionFlow, NewMessage(FbMessageWithBridgeAndStatus(id, type, bridge, status))).Times(1).WillRepeatedly(Invoke(PrintDebugMsg));
}

void TestConnectionFlowBase::expect_message(FeedbackMessage::Id id, FeedbackMessage::FeedbackType type, int bridgeListSize) {
    PrintExpectMsg(id);
    EXPECT_CALL(*_connectionFlow, NewMessage(FbMessageWithBridgeList(id, type, bridgeListSize))).Times(1).WillRepeatedly(Invoke(PrintDebugMsg));
}

void TestConnectionFlowBase::expect_no_actions() {
    EXPECT_CALL(*_factory, CreateSearcher()).Times(0);
    EXPECT_CALL(*_factory, CreateAuthenticator()).Times(0);

    for (auto searcher: *_searchers) {
        if (searcher) {
            EXPECT_CALL(*searcher, SearchNew(_)).Times(0);
            EXPECT_CALL(*searcher, SearchNew(_, _)).Times(0);
        }
    }
    for (auto authenticator: *_authenticators) {
        if (authenticator) {
            EXPECT_CALL(*authenticator, Authenticate(_, _, _)).Times(0);
        }
    }
    EXPECT_CALL(*_fullConfigRetriever, Execute(_, _, _)).Times(0);
}

void TestConnectionFlowBase::expect_on_searcher_search_new(bool bruteForce) {
    auto x = std::make_shared<MockBridgeSearcher>();
    _searchers->push_back(x);

    EXPECT_CALL(*_factory, CreateSearcher()).Times(1).WillOnce(Return(x));
    EXPECT_CALL(*x, SearchNew(bruteForce, _)).Times(1).WillOnce(SaveArg<1>(&x->SearchNewCallback));
}

shared_ptr<MockBridgeSearcher> TestConnectionFlowBase::get_last_searcher() {
    auto last = _searchers->size() - 1;
    return _searchers->at(last);
}

shared_ptr<MockBridgeAuthenticator> TestConnectionFlowBase::get_last_authenticator() {
    auto last = _authenticators->size() - 1;
    return _authenticators->at(last);
}

void TestConnectionFlowBase::expect_on_searcher_abort() {
    EXPECT_CALL(*get_last_searcher(), Abort()).Times(1);
}

void TestConnectionFlowBase::expect_on_authenticator_authenticate(int index, int times) {
    if (_authenticators->at(index) == nullptr) {
        _authenticators->at(index) = std::make_shared<MockBridgeAuthenticator>();
    }

    auto x = _authenticators->at(index);

    for (int i = 0; i < times; ++i) {
        EXPECT_CALL(*_factory, CreateAuthenticator()).InSequence(s).WillOnce(Return(x));

        EXPECT_CALL(*x, Authenticate(BridgeIpAddress(_bridges->at(index)->GetIpAddress()), _, _)).WillOnce(
            SaveArg<2>(&x->authenticate_callback)).RetiresOnSaturation();
    }
}

void TestConnectionFlowBase::expect_on_storage_accessor_load() {
    EXPECT_CALL(*_storageAccessor, Load(_)).Times(1).WillOnce(SaveArg<0>(&_storageAccessor->load_callback));
}

void TestConnectionFlowBase::expect_on_storage_accessor_save() {
    expect_message(FeedbackMessage::ID_START_SAVING, FeedbackMessage::FEEDBACK_TYPE_INFO);
    EXPECT_CALL(*_storageAccessor, Save(_, _)).Times(1).WillOnce(SaveArg<1>(&_storageAccessor->save_callback));
}

void TestConnectionFlowBase::expect_on_storage_accessor_save(BridgePtr bridge) {
    expect_message(FeedbackMessage::ID_START_SAVING, FeedbackMessage::FEEDBACK_TYPE_INFO);
    EXPECT_CALL(*_storageAccessor, Save(ActiveBridgeAttributes(bridge), _)).Times(1).WillOnce(
            SaveArg<1>(&_storageAccessor->save_callback));
}

void TestConnectionFlowBase::expect_on_storage_accessor_save(int numberOfBridges, std::string activeBridgeId) {
    expect_message(FeedbackMessage::ID_START_SAVING, FeedbackMessage::FEEDBACK_TYPE_INFO);
    EXPECT_CALL(*_storageAccessor, Save(NumberOfBridgesAndActiveBridgeId(numberOfBridges, activeBridgeId), _)).Times(1).WillOnce(
        SaveArg<1>(&_storageAccessor->save_callback));
}

void TestConnectionFlowBase::connect_starts_bridge_loading(bool background) {
    expect_message(FeedbackMessage::ID_USERPROCEDURE_STARTED, FeedbackMessage::FEEDBACK_TYPE_INFO);
    expect_message(FeedbackMessage::ID_START_LOADING, FeedbackMessage::FEEDBACK_TYPE_INFO);
    expect_on_storage_accessor_load();
    if (background) {
        _connectionFlow->ConnectToBridgeBackground();
    } else {
        _connectionFlow->ConnectToBridge();
    }
    _messageDispatcher->ExecutePendingActions();
}

void TestConnectionFlowBase::finish_search_with_no_bridges(shared_ptr<MockBridgeSearcher> searcher) {
    searcher->SearchNewCallback(std::make_shared<BridgeList>());
    _messageDispatcher->ExecutePendingActions();
}

void TestConnectionFlowBase::finish_search_with_one_bridge(shared_ptr<MockBridgeSearcher> searcher, int index) {
    BridgeListPtr list = std::make_shared<BridgeList>();
    list->push_back(_bridges->at(index));
    searcher->SearchNewCallback(list);
    _messageDispatcher->ExecutePendingActions();
}

void TestConnectionFlowBase::finish_search_with_three_bridges(shared_ptr<MockBridgeSearcher> searcher) {
    BridgeListPtr list = std::make_shared<BridgeList>();
    list->push_back(_bridges->at(0));
    list->push_back(_bridges->at(1));
    list->push_back(_bridges->at(2));
    searcher->SearchNewCallback(list);
    _messageDispatcher->ExecutePendingActions();
}

void TestConnectionFlowBase::finish_search_with_only_v1_bridge(shared_ptr<MockBridgeSearcher> searcher) {
    BridgeListPtr list = std::make_shared<BridgeList>();
    _bridges->at(0)->SetModelId("BSB001");
    list->push_back(_bridges->at(0));
    searcher->SearchNewCallback(list);
    _messageDispatcher->ExecutePendingActions();
}

void TestConnectionFlowBase::searching_with_three_bridges_found_starts_pushlink_on_all_three_bridges() {
    expect_message(FeedbackMessage::ID_FINISH_SEARCH_BRIDGES_FOUND, FeedbackMessage::FEEDBACK_TYPE_INFO);
    expect_message(FeedbackMessage::ID_START_AUTHORIZING, FeedbackMessage::FEEDBACK_TYPE_INFO);
    expect_message(FeedbackMessage::ID_PRESS_PUSH_LINK, FeedbackMessage::FEEDBACK_TYPE_USER);

    expect_on_authenticator_authenticate(0);
    expect_on_authenticator_authenticate(1);
    expect_on_authenticator_authenticate(2);

    finish_search_with_three_bridges(get_last_searcher());
}

void TestConnectionFlowBase::searching_with_one_bridge_found_starts_pushlink(int index) {
    expect_message(FeedbackMessage::ID_FINISH_SEARCH_BRIDGES_FOUND, FeedbackMessage::FEEDBACK_TYPE_INFO);
    expect_message(FeedbackMessage::ID_START_AUTHORIZING, FeedbackMessage::FEEDBACK_TYPE_INFO);
    expect_message(FeedbackMessage::ID_PRESS_PUSH_LINK, FeedbackMessage::FEEDBACK_TYPE_USER);
    expect_on_authenticator_authenticate(index);
    finish_search_with_one_bridge(get_last_searcher(), index);
}

void TestConnectionFlowBase::searching_background_with_one_bridge_found_finishes(int index) {
    expect_message(FeedbackMessage::ID_FINISH_SEARCH_BRIDGES_FOUND, FeedbackMessage::FEEDBACK_TYPE_INFO);
    expect_message(FeedbackMessage::ID_DONE_BRIDGE_FOUND, FeedbackMessage::FEEDBACK_TYPE_INFO);
    expect_message(FeedbackMessage::ID_USERPROCEDURE_FINISHED, FeedbackMessage::FEEDBACK_TYPE_INFO);
    finish_search_with_one_bridge(get_last_searcher(), index);
}

void TestConnectionFlowBase::searching_background_with_no_bridge_found_finishes() {
    expect_message(FeedbackMessage::ID_FINISH_SEARCHING_NO_BRIDGES_FOUND, FeedbackMessage::FEEDBACK_TYPE_INFO);
    expect_message(FeedbackMessage::ID_DONE_NO_BRIDGE_FOUND, FeedbackMessage::FEEDBACK_TYPE_INFO);
    expect_message(FeedbackMessage::ID_USERPROCEDURE_FINISHED, FeedbackMessage::FEEDBACK_TYPE_INFO);
    finish_search_with_no_bridges(get_last_searcher());
    _messageDispatcher->ExecutePendingActions();
}

void TestConnectionFlowBase::searching_background_with_only_v1_bridge_found_finishes() {
    expect_message(FeedbackMessage::ID_FINISH_SEARCHING_INVALID_BRIDGES_FOUND, FeedbackMessage::FEEDBACK_TYPE_INFO);
    expect_message(FeedbackMessage::ID_DONE_NO_BRIDGE_FOUND, FeedbackMessage::FEEDBACK_TYPE_INFO);
    expect_message(FeedbackMessage::ID_USERPROCEDURE_FINISHED, FeedbackMessage::FEEDBACK_TYPE_INFO);
    finish_search_with_only_v1_bridge(get_last_searcher());
    _messageDispatcher->ExecutePendingActions();
}

void TestConnectionFlowBase::searching_once_with_no_bridges_found_starts_new_bridge_search() {
    auto searcher = get_last_searcher();
    expect_message(FeedbackMessage::ID_FINISH_SEARCHING_NO_BRIDGES_FOUND, FeedbackMessage::FEEDBACK_TYPE_INFO);
    expect_message(FeedbackMessage::ID_START_SEARCHING, FeedbackMessage::FEEDBACK_TYPE_INFO);
    expect_on_searcher_search_new(true);
    finish_search_with_no_bridges(searcher);
    _messageDispatcher->ExecutePendingActions();
}

void TestConnectionFlowBase::searching_twice_with_no_bridges_found_finalizes() {
    searching_twice_with_no_bridges_found_finalizes(std::make_shared<Bridge>(std::make_shared<BridgeSettings>()), false);
}

void TestConnectionFlowBase::searching_twice_with_no_bridges_found_finalizes(BridgePtr bridge, bool existingBridge) {
    BridgeStatus status = BRIDGE_EMPTY;
    FeedbackMessage::Id messageId = FeedbackMessage::ID_NO_BRIDGE_FOUND;
    if (existingBridge) {
        status = BRIDGE_NOT_CONNECTED;
        messageId = FeedbackMessage::ID_BRIDGE_NOT_FOUND;
    }

    auto searcher = get_last_searcher();
    expect_message(FeedbackMessage::ID_FINISH_SEARCHING_NO_BRIDGES_FOUND, FeedbackMessage::FEEDBACK_TYPE_INFO);
    expect_message(FeedbackMessage::ID_DONE_ACTION_REQUIRED, FeedbackMessage::FEEDBACK_TYPE_INFO, bridge, status);
    expect_message(messageId, FeedbackMessage::FEEDBACK_TYPE_USER, bridge);
    expect_on_storage_accessor_save(bridge);
    finish_search_with_no_bridges(searcher);
    _messageDispatcher->ExecutePendingActions();
}

void TestConnectionFlowBase::authorize_bridge(int index) {
    _bridges->at(index)->SetIsAuthorized(true);
    _bridges->at(index)->SetUser("TestUser");
    _bridges->at(index)->SetClientKey("00000000000000000000000000000000");
}

void
TestConnectionFlowBase::finish_authorization_successfully_and_retrieve_full_config(int index, unsigned int numGroups, unsigned int numBridges) {
    finish_authorization_successfully(index);
    retrieve_full_config(index, numGroups, numBridges);
}

void TestConnectionFlowBase::finish_authorization_successfully(int index) {
    authorize_bridge(index);

    expect_message(FeedbackMessage::ID_FINISH_AUTHORIZING_AUTHORIZED, FeedbackMessage::FEEDBACK_TYPE_INFO);		
    expect_initiate_full_config_retrieval();

    _authenticators->at(index)->authenticate_callback(_bridges->at(index));
    _messageDispatcher->ExecutePendingActions();
}

void TestConnectionFlowBase::expect_storage_accessor_load_return_data() {
    expect_message(FeedbackMessage::ID_START_LOADING, FeedbackMessage::FEEDBACK_TYPE_INFO);
    expect_message(FeedbackMessage::ID_FINISH_LOADING_BRIDGE_CONFIGURED, FeedbackMessage::FEEDBACK_TYPE_INFO);
    EXPECT_CALL(*_storageAccessor, Load(_)).Times(1).WillOnce(InvokeArgument<0>(OPERATION_SUCCESS, _persistentData));
}

void TestConnectionFlowBase::expect_small_config_retrieval_return_data(int bridge_index) {
    expect_small_config_retrieval_return_data(_bridges->at(bridge_index));
}

void TestConnectionFlowBase::expect_small_config_retrieval_return_data(BridgePtr bridge) {
    expect_message(FeedbackMessage::ID_START_RETRIEVING_SMALL, FeedbackMessage::FEEDBACK_TYPE_INFO);
    expect_message(FeedbackMessage::ID_FINISH_RETRIEVING_SMALL, FeedbackMessage::FEEDBACK_TYPE_INFO);
    EXPECT_CALL(*_smallConfigRetriever, Execute(_, _, _)).Times(1).WillOnce(DoAll(InvokeArgument<1>(OPERATION_SUCCESS, bridge), Return(true)));
}

void TestConnectionFlowBase::expect_initiate_small_config_retrieval() {
    expect_message(FeedbackMessage::ID_START_RETRIEVING_SMALL, FeedbackMessage::FEEDBACK_TYPE_INFO);
    expect_on_small_config_retriever_execute();
}

void TestConnectionFlowBase::expect_initiate_full_config_retrieval() {
    expect_message(FeedbackMessage::ID_START_RETRIEVING, FeedbackMessage::FEEDBACK_TYPE_INFO);
    expect_on_full_config_retriever_execute();
}

void TestConnectionFlowBase::retrieve_full_config(int index, unsigned int numGroups, unsigned int numBridges) {
    set_bridge_in_full_config(numGroups);

    if (numGroups == 0) {
        expect_message(FeedbackMessage::ID_FINISH_RETRIEVING_ACTION_REQUIRED, FeedbackMessage::FEEDBACK_TYPE_INFO, _bridges->at(index), BRIDGE_NO_GROUP_AVAILABLE);
        expect_message(FeedbackMessage::ID_DONE_ACTION_REQUIRED, FeedbackMessage::FEEDBACK_TYPE_INFO, _bridges->at(index), BRIDGE_NO_GROUP_AVAILABLE);
        expect_message(FeedbackMessage::ID_NO_GROUP_AVAILABLE, FeedbackMessage::FEEDBACK_TYPE_USER, _bridges->at(index));
    } else if (numGroups == 1) {
        expect_message(FeedbackMessage::ID_FINISH_RETRIEVING_READY_TO_START, FeedbackMessage::FEEDBACK_TYPE_INFO, _bridges->at(index), BRIDGE_READY);
    } else if (numGroups > 1) {
        expect_message(FeedbackMessage::ID_FINISH_RETRIEVING_ACTION_REQUIRED, FeedbackMessage::FEEDBACK_TYPE_INFO, _bridges->at(index), BRIDGE_INVALID_GROUP_SELECTED);
        expect_message(FeedbackMessage::ID_DONE_ACTION_REQUIRED, FeedbackMessage::FEEDBACK_TYPE_INFO, _bridges->at(index), BRIDGE_INVALID_GROUP_SELECTED);
        expect_message(FeedbackMessage::ID_SELECT_GROUP, FeedbackMessage::FEEDBACK_TYPE_USER, _bridges->at(index), BRIDGE_INVALID_GROUP_SELECTED);
    }

    expect_on_storage_accessor_save(numBridges, _bridges->at(index)->GetId());

    _fullConfigRetriever->RetrieveCallback(OPERATION_SUCCESS, _fullConfigRetriever->Bridge);
    _messageDispatcher->ExecutePendingActions();
}

void TestConnectionFlowBase::checkUpdateMessages(bool connected, bool groupsUpdated,bool bridgeChanged) {
    if (connected) {
        expect_message(FeedbackMessage::ID_BRIDGE_CONNECTED, FeedbackMessage::FEEDBACK_TYPE_INFO);
    }
    if (bridgeChanged) {
        expect_message(FeedbackMessage::ID_BRIDGE_CHANGED, FeedbackMessage::FEEDBACK_TYPE_INFO);
    }
    if (groupsUpdated) {
        expect_message(FeedbackMessage::ID_GROUPLIST_UPDATED, FeedbackMessage::FEEDBACK_TYPE_INFO);
    }
}

void TestConnectionFlowBase::finish_with_stream_start(BridgePtr bridge, bool connected, bool groupsUpdated, bool streamConnected, bool bridgeChanged) {
    expect_message(FeedbackMessage::ID_FINISH_SAVING_SAVED, FeedbackMessage::FEEDBACK_TYPE_INFO);
    expect_message(FeedbackMessage::ID_START_ACTIVATING, FeedbackMessage::FEEDBACK_TYPE_INFO);
    expect_message(FeedbackMessage::ID_FINISH_ACTIVATING_ACTIVE, FeedbackMessage::FEEDBACK_TYPE_INFO, bridge, BRIDGE_STREAMING);
    if (streamConnected) {
        expect_message(FeedbackMessage::ID_STREAMING_CONNECTED, FeedbackMessage::FEEDBACK_TYPE_INFO);
    }
    expect_message(FeedbackMessage::ID_DONE_COMPLETED, FeedbackMessage::FEEDBACK_TYPE_INFO);
    checkUpdateMessages(connected, groupsUpdated, bridgeChanged);
    expect_message(FeedbackMessage::ID_USERPROCEDURE_FINISHED, FeedbackMessage::FEEDBACK_TYPE_INFO);

    if (_settings->UseRenderThread()) {
        EXPECT_CALL(*_stream, StartWithRenderThread(bridge)).WillOnce(Invoke(ActivateBridge));
    } else {
        EXPECT_CALL(*_stream, Start(bridge)).WillOnce(Invoke(ActivateBridge));
    }
    _storageAccessor->save_callback(OPERATION_SUCCESS);
    _messageDispatcher->ExecutePendingActions();
}

void TestConnectionFlowBase::finish_without_stream_start(bool completed, bool connected, bool groupsUpdated, bool bridgeChanged) {
    expect_message(FeedbackMessage::ID_FINISH_SAVING_SAVED, FeedbackMessage::FEEDBACK_TYPE_INFO);
    if (completed) {
        expect_message(FeedbackMessage::ID_DONE_COMPLETED, FeedbackMessage::FEEDBACK_TYPE_INFO);
    }
    checkUpdateMessages(connected, groupsUpdated, bridgeChanged);
    expect_message(FeedbackMessage::ID_USERPROCEDURE_FINISHED, FeedbackMessage::FEEDBACK_TYPE_INFO);
    _storageAccessor->save_callback(OPERATION_SUCCESS);
    _messageDispatcher->ExecutePendingActions();
}

void TestConnectionFlowBase::finish(BridgePtr bridge) {
    if (_settings->AutoStartAtConnection()) {
        finish_with_stream_start(bridge);
    } else {
        finish_without_stream_start(true);
    }
}

void TestConnectionFlowBase::abort_finalizes() {
    expect_message(FeedbackMessage::ID_DONE_ABORTED, FeedbackMessage::FEEDBACK_TYPE_INFO);
    expect_message(FeedbackMessage::ID_USERPROCEDURE_FINISHED, FeedbackMessage::FEEDBACK_TYPE_INFO);
    _connectionFlow->Abort();
    _messageDispatcher->ExecutePendingActions();
}

void TestConnectionFlowBase::searching_with_abort_finalizes() {
    expect_on_searcher_abort();
    abort_finalizes();
}

void TestConnectionFlowBase::select_group_existing(int bridgeindex, string groupid) {
    expect_message(FeedbackMessage::ID_USERPROCEDURE_STARTED, FeedbackMessage::FEEDBACK_TYPE_INFO);
    expect_on_storage_accessor_save();
    _connectionFlow->SelectGroup(groupid);
    _messageDispatcher->ExecutePendingActions();
}
