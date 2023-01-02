#include <huestream/config/Config.h>
#include "gtest/gtest.h"
#include "huestream/common/data/Bridge.h"
#include "huestream/effect/animation/animations/SequenceAnimation.h"
#include "huestream/config/ObjectBuilder.h"

using namespace huestream;
using std::static_pointer_cast;

class TestBridge : public testing::Test {
protected:
    BridgePtr bridge;

    virtual void SetUp() {
        Serializable::SetObjectBuilder(std::make_shared<ObjectBuilder>(nullptr));
        bridge = std::make_shared<Bridge>(std::make_shared<BridgeSettings>());
    }

    virtual void TearDown() {
        Serializable::SetObjectBuilder(std::make_shared<ObjectBuilder>(nullptr));
    }

    void SetConnectableProperties() {
        bridge->SetId("dsfsdf");
        bridge->SetIpAddress("dsfsdf");
        bridge->SetUser("dsfsdf");
        bridge->SetIsAuthorized(true);
        bridge->SetModelId("BSB002");
    }

    void SetValidBaseProperties() {
        SetConnectableProperties();
        bridge->SetApiversion("1.24.0");
        bridge->SetSwversion("1940094000");
        bridge->SetClientKey("DD129216F1A50E5D1C0CB356325745F2");
        bridge->SetIsValidIp(true);
        bridge->SetMaxNoStreamingSessions(1);
    }

    void SetOneGroup(std::string id) {
        auto group = std::make_shared<Group>();
        group->SetId(id);
        auto groupsWithOneGroup = std::make_shared<GroupList>();
        groupsWithOneGroup->push_back(group);
        bridge->SetGroups(groupsWithOneGroup);
    }

    void SetTwoGroups(std::string id1, std::string id2) {
        auto group1 = std::make_shared<Group>();
        group1->SetId(id1);
        auto group2 = std::make_shared<Group>();
        group2->SetId(id2);
        auto groupsWithTwoGroups = std::make_shared<GroupList>();
        groupsWithTwoGroups->push_back(group1);
        groupsWithTwoGroups->push_back(group2);
        bridge->SetGroups(groupsWithTwoGroups);
    }

    void SetValidPropertiesUnselectedGroup(std::string id) {
        SetValidBaseProperties();
        SetOneGroup(id);
    }

    void SetValidPropertiesUnselectedGroups(std::string id1, std::string id2) {
        SetValidBaseProperties();
        SetTwoGroups(id1, id2);
    }

    void SetValidProperties() {
        SetValidPropertiesUnselectedGroup("3");
        bridge->SetSelectedGroup("3");
    }
};

TEST_F(TestBridge, IsEmpty) {
    ASSERT_TRUE(bridge->IsEmpty());
    ASSERT_FALSE(bridge->IsFound());
    ASSERT_TRUE(bridge->GetStatus() == BRIDGE_EMPTY);
    ASSERT_TRUE(bridge->GetStatusTag() == bridge->statusTagMap[BRIDGE_EMPTY]);

    bridge->SetIpAddress("192.168.1.1");
    ASSERT_FALSE(bridge->IsEmpty());
    ASSERT_FALSE(bridge->IsFound());
    ASSERT_TRUE(bridge->GetStatus() == BRIDGE_NOT_FOUND);

    bridge->SetId("123456");
    ASSERT_FALSE(bridge->IsEmpty());
    ASSERT_TRUE(bridge->IsFound());
}

TEST_F(TestBridge, IsValidModelId) {
    ASSERT_FALSE(bridge->IsValidModelId());

    bridge->SetModelId("BSB001");
    ASSERT_FALSE(bridge->IsValidModelId());

    bridge->SetModelId("BSB002");
    ASSERT_TRUE(bridge->IsValidModelId());

    bridge->SetModelId("BxB002");
    ASSERT_FALSE(bridge->IsValidModelId());

    bridge->SetModelId("BSB003");
    ASSERT_TRUE(bridge->IsValidModelId());
}

TEST_F(TestBridge, IsValidClientKey) {
    bridge->SetClientKey("");
    ASSERT_FALSE(bridge->IsValidClientKey());

    bridge->SetClientKey("invalid");
    ASSERT_FALSE(bridge->IsValidClientKey());

    bridge->SetClientKey("DD129216F1A50E5D1C0CB356325745F2");
    ASSERT_TRUE(bridge->IsValidClientKey());

    bridge->SetClientKey("012345678901234567890123456789012");
    ASSERT_FALSE(bridge->IsValidClientKey());

    bridge->SetClientKey("0123456789012345678901234567890");
    ASSERT_FALSE(bridge->IsValidClientKey());
}

TEST_F(TestBridge, IsStreamable) {
    SetConnectableProperties();
    ASSERT_TRUE(bridge->IsConnectable());
    ASSERT_FALSE(bridge->IsReadyToStream());

    SetConnectableProperties();
    bridge->SetModelId("BSB001");
    ASSERT_FALSE(bridge->IsConnectable());
    ASSERT_FALSE(bridge->IsReadyToStream());
    ASSERT_TRUE(bridge->GetStatus() == BRIDGE_INVALID_MODEL);

    SetValidProperties();
    ASSERT_TRUE(bridge->IsReadyToStream());
    ASSERT_TRUE(bridge->GetStatus() == BRIDGE_READY);

    SetValidProperties();
    bridge->SetIsValidIp(false);
    ASSERT_FALSE(bridge->IsReadyToStream());
    ASSERT_TRUE(bridge->GetStatus() == BRIDGE_NOT_CONNECTED);

    SetValidProperties();
    bridge->SetApiversion("1.21.0");
    bridge->SetSwversion("1940094000");
    ASSERT_FALSE(bridge->IsReadyToStream());
    ASSERT_TRUE(bridge->GetStatus() == BRIDGE_INVALID_VERSION);

    SetValidProperties();
    bridge->SetSelectedGroup("0");
    ASSERT_FALSE(bridge->IsReadyToStream());
    ASSERT_TRUE(bridge->GetStatus() == BRIDGE_INVALID_GROUP_SELECTED);

    SetValidProperties();
    auto groupsWithNo = std::make_shared<GroupList>();
    bridge->SetGroups(groupsWithNo);
    ASSERT_FALSE(bridge->IsReadyToStream());
    ASSERT_TRUE(bridge->GetStatus() == BRIDGE_NO_GROUP_AVAILABLE);

    SetValidProperties();
    auto groupsWithOne4 = std::make_shared<GroupList>();
    auto group4 = std::make_shared<Group>();
    group4->SetId("4");
    groupsWithOne4->push_back(group4);
    bridge->SetGroups(groupsWithOne4);
    ASSERT_FALSE(bridge->IsReadyToStream());
    ASSERT_TRUE(bridge->GetStatus() == BRIDGE_INVALID_GROUP_SELECTED);

    SetValidProperties();
    bridge->SetClientKey("");
    ASSERT_FALSE(bridge->IsReadyToStream());
    ASSERT_TRUE(bridge->GetStatus() == BRIDGE_INVALID_CLIENTKEY);

    SetValidProperties();
    bridge->SetIsBusy(true);
    ASSERT_FALSE(bridge->IsReadyToStream());
    ASSERT_TRUE(bridge->GetStatus() == BRIDGE_BUSY);

    ASSERT_TRUE(bridge->HasEverBeenAuthorizedForStreaming());
}

TEST_F(TestBridge, IsStreaming) {
    SetValidPropertiesUnselectedGroups("3", "5");
    bridge->SelectGroup("5");
    ASSERT_EQ(bridge->GetStatus(), BRIDGE_READY);

    ASSERT_FALSE(bridge->IsStreaming());

    bridge->GetGroup()->SetActive(true);
    bridge->GetGroup()->SetOwner("someoneelse");
    ASSERT_FALSE(bridge->IsStreaming());

    bridge->GetGroup()->SetOwner(bridge->GetUser());
    ASSERT_TRUE(bridge->IsStreaming());

    bridge->SelectGroup("3");
    ASSERT_FALSE(bridge->IsStreaming());
}

TEST_F(TestBridge, IsConnectable) {
    ASSERT_FALSE(bridge->IsConnectable());

    SetConnectableProperties();
    ASSERT_TRUE(bridge->IsConnectable());

    SetConnectableProperties();
    bridge->SetId("");
    ASSERT_FALSE(bridge->IsConnectable());

    SetConnectableProperties();
    bridge->SetIpAddress("");
    ASSERT_FALSE(bridge->IsConnectable());

    SetConnectableProperties();
    bridge->SetUser("");
    ASSERT_FALSE(bridge->IsConnectable());

    SetConnectableProperties();
    bridge->SetIsAuthorized(false);
    ASSERT_FALSE(bridge->IsConnectable());

    SetConnectableProperties();
    bridge->SetModelId("BSB001");
    ASSERT_FALSE(bridge->IsConnectable());
}

TEST_F(TestBridge, IsValidApiVersion) {
    ASSERT_TRUE(bridge->GetApiversion().empty());
    ASSERT_FALSE(bridge->IsValidApiVersion());

    bridge->SetApiversion("1.24.0");
    ASSERT_TRUE(bridge->IsValidApiVersion());

    bridge->SetApiversion("1.24.1");
    ASSERT_TRUE(bridge->IsValidApiVersion());

    bridge->SetApiversion("1.25.0");
    ASSERT_TRUE(bridge->IsValidApiVersion());

    bridge->SetApiversion("2.0.0");
    ASSERT_TRUE(bridge->IsValidApiVersion());

    bridge->SetApiversion("0.0.0");
    ASSERT_FALSE(bridge->IsValidApiVersion());

    bridge->SetApiversion("1.21.9");
    ASSERT_FALSE(bridge->IsValidApiVersion());
}

TEST_F(TestBridge, StreamingSessions) {
    SetValidProperties();
    ASSERT_EQ(bridge->GetMaxNoStreamingSessions(), 1);
    ASSERT_EQ(bridge->GetCurrentNoStreamingSessions(), 0);
    bridge->GetGroup()->SetActive(true);
    bridge->GetGroup()->SetOwner("fdfsdaf");
    ASSERT_EQ(bridge->GetMaxNoStreamingSessions(), 1);
    ASSERT_EQ(bridge->GetCurrentNoStreamingSessions(), 1);
}

TEST_F(TestBridge, Serialize) {
    bridge->SetId("00:11:22:33:44");
    bridge->SetUser("HJD77jsjs-7883kkKS@");
    bridge->SetClientKey("DD129216F1A50E5D1C0CB356325745F2");
    {
        auto group1 = std::make_shared<Group>();
        group1->SetId("123");
        group1->SetName("My entertainment setup1");
        group1->AddLight("22", 0.1, 0.1);
        group1->AddLight("12", 0.3, 0.2);

        auto group2 = std::make_shared<Group>();
        group2->SetId("13");
        group2->SetName("My entertainment setup2");
        group2->AddLight("11", 0.4, 0.4);
        group2->AddLight("9", 0.4, 0.2);

        auto groups = std::make_shared<GroupList>();
        groups->push_back(group1);
        groups->push_back(group2);
        bridge->SetGroups(groups);
    }
    bridge->SetSelectedGroup("123");
    bridge->SetIsValidIp(true);
    bridge->SetIsAuthorized(true);
    bridge->SetIpAddress("192.168.1.1");

    JSONNode node;
    bridge->Serialize(&node);
    auto b = static_pointer_cast<Bridge>(Bridge::DeserializeFromJson(&node));


    ASSERT_EQ("123", b->GetSelectedGroup());
    ASSERT_EQ(true, b->IsValidIp());
    ASSERT_EQ(true, b->IsAuthorized());
    ASSERT_EQ("192.168.1.1", b->GetIpAddress());
    ASSERT_EQ("00:11:22:33:44", b->GetId());
    ASSERT_EQ("HJD77jsjs-7883kkKS@", b->GetUser());
    ASSERT_EQ("123", b->GetGroups()->at(0)->GetId());
    ASSERT_EQ("DD129216F1A50E5D1C0CB356325745F2", b->GetClientKey());
    ASSERT_EQ("My entertainment setup1", b->GetGroups()->at(0)->GetName());
    ASSERT_EQ(2, b->GetGroups()->at(0)->GetLights()->size());
    ASSERT_EQ("22", b->GetGroups()->at(0)->GetLights()->at(0)->GetId());
    ASSERT_EQ(0.1, b->GetGroups()->at(0)->GetLights()->at(0)->GetPosition().GetX());
    ASSERT_EQ(0.1, b->GetGroups()->at(0)->GetLights()->at(0)->GetPosition().GetY());
    ASSERT_EQ("12", b->GetGroups()->at(0)->GetLights()->at(1)->GetId());
    ASSERT_EQ(0.3, b->GetGroups()->at(0)->GetLights()->at(1)->GetPosition().GetX());
    ASSERT_EQ(0.2, b->GetGroups()->at(0)->GetLights()->at(1)->GetPosition().GetY());

    ASSERT_EQ("13", b->GetGroups()->at(1)->GetId());
    ASSERT_EQ("My entertainment setup2", b->GetGroups()->at(1)->GetName());
    ASSERT_EQ("11", b->GetGroups()->at(1)->GetLights()->at(0)->GetId());
    ASSERT_EQ(0.4, b->GetGroups()->at(1)->GetLights()->at(0)->GetPosition().GetX());
    ASSERT_EQ(0.4, b->GetGroups()->at(1)->GetLights()->at(0)->GetPosition().GetY());
    ASSERT_EQ("9", b->GetGroups()->at(1)->GetLights()->at(1)->GetId());
    ASSERT_EQ(0.4, b->GetGroups()->at(1)->GetLights()->at(1)->GetPosition().GetX());
    ASSERT_EQ(0.2, b->GetGroups()->at(1)->GetLights()->at(1)->GetPosition().GetY());
}

TEST_F(TestBridge, SelectFromOneGroup) {
    auto groupId = "4";

    SetValidPropertiesUnselectedGroup(groupId);
    ASSERT_TRUE(bridge->GetGroup() == nullptr);
    ASSERT_TRUE(bridge->SelectGroupIfOnlyOneOption());
    ASSERT_TRUE(bridge->GetGroup() != nullptr);
    ASSERT_TRUE(bridge->GetSelectedGroup() == groupId);
}

TEST_F(TestBridge, SelectFromMultipleGroups) {
    auto groupId1 = "5";
    auto groupId2 = "7";

    SetValidPropertiesUnselectedGroups(groupId1, groupId2);
    ASSERT_FALSE(bridge->SelectGroupIfOnlyOneOption());
    ASSERT_TRUE(bridge->GetGroup() == nullptr);
    ASSERT_FALSE(bridge->SelectGroup("9"));
    ASSERT_TRUE(bridge->GetGroup() == nullptr);

    ASSERT_TRUE(bridge->SelectGroup(groupId1));
    ASSERT_TRUE(bridge->GetGroup() != nullptr);
    ASSERT_TRUE(bridge->GetSelectedGroup() == groupId1);

    ASSERT_TRUE(bridge->SelectGroup(groupId2));
    ASSERT_TRUE(bridge->GetGroup() != nullptr);
    ASSERT_TRUE(bridge->GetSelectedGroup() == groupId2);
}

TEST_F(TestBridge, SerializeDeserializeCompact) {
    BridgePtr bridge_d = std::make_shared<Bridge>(std::make_shared<BridgeSettings>());
    bridge->SetId("00:11:22:33:44");
    bridge->SetUser("HJD77jsjs-7883kkKS@");
    bridge->SetModelId("BSB002");
    bridge->SetApiversion("1.17.0");
    bridge->SetClientKey("DD129216F1A50E5D1C0CB356325745F2");
    bridge->SetIsValidIp(true);
    bridge->SetIsAuthorized(true);
    bridge->SetIpAddress("192.168.1.1");

    std::string json = bridge->SerializeCompact();

    bridge_d->DeserializeCompact(json);

    ASSERT_EQ(bridge->GetId(),              bridge_d->GetId());
    ASSERT_EQ(bridge->GetUser(),            bridge_d->GetUser());
    ASSERT_EQ(bridge->GetModelId(),         bridge_d->GetModelId());
    ASSERT_EQ(bridge->GetApiversion(),      bridge_d->GetApiversion());
    ASSERT_EQ(bridge->GetClientKey(),       bridge_d->GetClientKey());
    ASSERT_EQ(bridge->IsValidIp(),          bridge_d->IsValidIp());
    ASSERT_EQ(bridge->IsAuthorized(),       bridge_d->IsAuthorized());
    ASSERT_EQ(bridge->GetIpAddress(),       bridge_d->GetIpAddress());
}

TEST_F(TestBridge, GetVariousUrlsNoPortSet) {
    SetValidPropertiesUnselectedGroup("0");
    bridge->SetIpAddress("127.0.0.1");
    bridge->SetUser("rocky");
    bridge->SelectGroup("0");

    const std::string urlApiRoot = "https://127.0.0.1/api/";
    const std::string baseUrl = urlApiRoot + "rocky/";
    const std::string selectedGroupUrl = baseUrl + "groups/0";

    EXPECT_EQ(urlApiRoot, bridge->GetApiRootUrl());
    EXPECT_EQ(baseUrl, bridge->GetBaseUrl());
    EXPECT_EQ(selectedGroupUrl, bridge->GetSelectedGroupUrl());
}

TEST_F(TestBridge, ProxyReachability) {
    SetValidPropertiesUnselectedGroup("0");
    ASSERT_FALSE(bridge->IsProxyNodeUnreachable());

    bridge->SelectGroup("0");
    GroupProxyNode reachableProxy = { "", "", "", "", true };
    bridge->GetGroup()->SetProxyNode(reachableProxy);
    ASSERT_FALSE(bridge->IsProxyNodeUnreachable());

    GroupProxyNode unreachableProxy = { "", "", "", "", false };
    bridge->GetGroup()->SetProxyNode(unreachableProxy);
    ASSERT_TRUE(bridge->IsProxyNodeUnreachable());
}

TEST_F(TestBridge, UseSslHttpsPortSet) {
    bridge->SetIpAddress("127.0.0.1");
    bridge->SetSslPort("2222");

    const std::string urlApiRoot = "https://127.0.0.1:2222/api/";
    EXPECT_EQ(urlApiRoot, bridge->GetApiRootUrl());
}

TEST_F(TestBridge, UseSslHttpsPortNotSet) {
    bridge->SetIpAddress("127.0.0.1");

    const std::string urlApiRoot = "https://127.0.0.1/api/";
    EXPECT_EQ(urlApiRoot, bridge->GetApiRootUrl());
}

/******************************************************************************/
/*                                 END OF FILE                                */
/******************************************************************************/
