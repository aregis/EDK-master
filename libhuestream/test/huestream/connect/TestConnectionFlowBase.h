#ifndef LIBHUESTREAM_TESTCONNECTIONFLOWBASE_H
#define LIBHUESTREAM_TESTCONNECTIONFLOWBASE_H


#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <memory>
#include <huestream/connect/ConnectionFlow.h>
#include <huestream/config/Config.h>
#include "test/huestream/_mock/MockBridgeAuthenticator.h"
#include "test/huestream/_mock/MockBridgeSearcher.h"
#include "TestableConnectionFlow.h"
#include "test/huestream/_mock/MockConnectionFlowFactory.h"
#include "test/huestream/_mock/MockBridgeStorageAccessor.h"
#include "test/huestream/_stub/StubMessageDispatcher.h"
#include "test/huestream/_mock/MockConfigRetriever.h"
#include "test/huestream/_mock/MockStream.h"
#include "test/huestream/_mock/MockScheduler.h"
#include "support/scheduler/SchedulerFactory.h"

using namespace testing;
using namespace huestream;

class TestConnectionFlowBase : public testing::TestWithParam<int> {
public:
    shared_ptr<Config> _config;
    shared_ptr<AppSettings> _settings;
    shared_ptr<BridgeList> _bridges;
    std::shared_ptr<TestableConnectionFlow> _connectionFlow;
    shared_ptr<MockConnectionFlowFactory> _factory;
    std::shared_ptr<MockScheduler> _scheduler;
    shared_ptr<vector<shared_ptr<MockBridgeSearcher>>> _searchers;
    shared_ptr<vector<shared_ptr<MockBridgeAuthenticator>>> _authenticators;
    shared_ptr<StubMessageDispatcher> _messageDispatcher;
    shared_ptr<MockBridgeStorageAccessor> _storageAccessor;
    shared_ptr<MockStream> _stream;
    shared_ptr<HueStreamData> _persistentData;

    Sequence s;

    void SetUp() override;

    void TearDown() override;

    bool int2bool(int param);

    void expect_message(FeedbackMessage::Id id, FeedbackMessage::FeedbackType type);

    void expect_message(FeedbackMessage::Id id, FeedbackMessage::FeedbackType type, BridgePtr bridge);

    void expect_message(FeedbackMessage::Id id, FeedbackMessage::FeedbackType type, BridgePtr bridge, BridgeStatus status);

    void expect_message(FeedbackMessage::Id id, FeedbackMessage::FeedbackType type, int bridgeListSize);

    void expect_no_actions();

    void expect_on_searcher_search_new(bool bruteForce);

    void expect_on_searcher_abort();

    void expect_on_authenticator_authenticate(int index, int times = 1);

    void expect_on_storage_accessor_load();

    void connect_starts_bridge_loading(bool background = false);

    void finish_search_with_no_bridges(shared_ptr<MockBridgeSearcher> searcher);

    void finish_search_with_one_bridge(shared_ptr<MockBridgeSearcher> searcher, int index);

    void finish_search_with_three_bridges(shared_ptr<MockBridgeSearcher> searcher);

    void finish_search_with_only_v1_bridge(shared_ptr<MockBridgeSearcher> searcher);

    void searching_with_three_bridges_found_starts_pushlink_on_all_three_bridges();

    void searching_with_one_bridge_found_starts_pushlink(int index);

    void searching_background_with_one_bridge_found_finishes(int index = 0);

    void searching_background_with_no_bridge_found_finishes();

    void searching_background_with_only_v1_bridge_found_finishes();

    void searching_once_with_no_bridges_found_starts_new_bridge_search();

    void searching_twice_with_no_bridges_found_finalizes();

    void searching_twice_with_no_bridges_found_finalizes(BridgePtr bridge, bool existingBridge);

    void authorize_bridge(int index);

    void finish_authorization_successfully_and_retrieve_full_config(int index, unsigned int numGroups = 1, unsigned int numBridges = 1);

    void finish_authorization_successfully(int index);

    void expect_storage_accessor_load_return_data();

    void expect_small_config_retrieval_return_data(int bridge_index);

    void expect_small_config_retrieval_return_data(BridgePtr bridge);

    void expect_initiate_small_config_retrieval();

    void expect_initiate_full_config_retrieval();

    void retrieve_full_config(int index, unsigned int numGroups, unsigned int numBridges = 1);

    void checkUpdateMessages(bool connected, bool groupsUpdated, bool bridgeChanged);

    void finish(BridgePtr bridge);

    void finish_without_stream_start(bool completed = true, bool connected = true, bool groupsUpdated = true, bool bridgeChanged = true);

    void finish_with_stream_start(BridgePtr bridge, bool connected = true, bool groupsUpdated = true, bool streamConnected = true, bool bridgeChanged = true);

    void abort_finalizes();

    void searching_with_abort_finalizes();

    void select_group_existing(int bridgeindex, std::string groupid);

    shared_ptr<MockBridgeSearcher> get_last_searcher();

    shared_ptr<MockBridgeAuthenticator> get_last_authenticator();

    shared_ptr<MockConfigRetriever> _smallConfigRetriever;

    shared_ptr<MockConfigRetriever> _fullConfigRetriever;

    void expect_on_storage_accessor_save();

    void expect_on_storage_accessor_save(BridgePtr bridge);

    void expect_on_storage_accessor_save(int numberOfBridges, std::string activeBridgeId);

    void expect_on_small_config_retriever_execute() const {
        EXPECT_CALL(*_smallConfigRetriever, Execute(_, _, _))
                .Times(1)
                .WillOnce(DoAll(
                        SaveArg<0>(&_smallConfigRetriever->Bridge),
                        SaveArg<1>(&_smallConfigRetriever->RetrieveCallback),
												SaveArg<2>(&_smallConfigRetriever->Feedback),
                        Return(true)));
    }

    void expect_on_full_config_retriever_execute() const {
        EXPECT_CALL(*_fullConfigRetriever, Execute(_, _, _))
                .Times(1)
                .WillOnce(DoAll(
                        SaveArg<0>(&_fullConfigRetriever->Bridge),
                        SaveArg<1>(&_fullConfigRetriever->RetrieveCallback),
												SaveArg<2>(&_smallConfigRetriever->Feedback),
                        Return(true)));
    }

    void set_bridge_in_full_config(unsigned int numGroups) const {
        if (_fullConfigRetriever->Bridge == nullptr) {
            return;
        }
        ASSERT_TRUE(numGroups <= 2);

        _fullConfigRetriever->Bridge->SetIsValidIp(true);
        _fullConfigRetriever->Bridge->SetModelId("BSB002");
        _fullConfigRetriever->Bridge->SetApiversion("1.24.0");
        _fullConfigRetriever->Bridge->SetSwversion("1940094000");
        auto groups = std::make_shared<GroupList>();

        if (numGroups >= 1) {
            auto group1 = std::make_shared<Group>();
            group1->SetId("12");
            group1->SetName("My Entertainment Group1");
            group1->AddLight("1", 0.5, 0.4);
            group1->AddLight("2", 0.4, 0.2);
            groups->push_back(group1);
        }
        if (numGroups >= 2) {
            auto group2 = std::make_shared<Group>();
            group2->SetId("13");
            group2->SetName("My Entertainment Group2");
            group2->AddLight("3", 0.5, 0.4);
            group2->AddLight("4", 0.4, 0.2);
            groups->push_back(group2);
        }
        _fullConfigRetriever->Bridge->SetGroups(groups);
    }

    void set_bridge_not_authorized_for_full_config() const {
        if (_fullConfigRetriever->Bridge == nullptr) {
            return;
        }
        _fullConfigRetriever->Bridge->SetIsAuthorized(false);
    }

    void set_bridge_not_found_for_full_config() const {
        if (_fullConfigRetriever->Bridge == nullptr) {
            return;
        }
        _fullConfigRetriever->Bridge->SetIsValidIp(false);
    }

    static bool ActivateBridge(BridgePtr bridge) {
        if (bridge->IsValidGroupSelected()) {
            bridge->GetGroup()->SetActive(true);
            bridge->GetGroup()->SetOwner(bridge->GetUser());
        }
        return true;
    }

    static bool FailActivateBridgeBusy(BridgePtr bridge) {
        bridge->SetIsBusy(true);
        return false;
    }

    static void DeactivateBridge(BridgePtr bridge) {
        bridge->GetGroup()->SetActive(false);
    }

    static void PrintDebugMsg(const FeedbackMessage &m) {
        //change below comment for easier debugging on connectionflow tests
        (void)m;
        //std::cout << m.GetDebugMessage() << std::endl;
    }

    static void PrintExpectMsg(const FeedbackMessage::Id id) {
        //change below comment for easier debugging on connectionflow tests
        FeedbackMessage m(FeedbackMessage::REQUEST_TYPE_INTERNAL, id, nullptr);
        //std::cout << "EXPECT: " << m.GetTag() << std::endl;
    }
};

#endif  // LIBHUESTREAM_TESTCONNECTIONFLOWBASE_H
