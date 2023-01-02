#include <huestream/config/ObjectBuilder.h>
#include "gtest/gtest.h"
#include "huestream/common/data/Group.h"

using namespace huestream;

class TestGroup : public testing::Test {
protected:
    Group *pGroup;

    virtual void SetUp() {
        Serializable::SetObjectBuilder(std::make_shared<ObjectBuilder>(nullptr));
        pGroup = new Group();
    }

    virtual void TearDown() {
        Serializable::SetObjectBuilder(std::make_shared<ObjectBuilder>(nullptr));
        delete pGroup;
    }
};


TEST_F(TestGroup, Serialize) {
    auto groupName = "My Entertainment group";
    auto lightName = "My light";
    auto lightModel = "LTW010";

    pGroup->SetId("12");
    pGroup->SetName(groupName);
    pGroup->SetClassType(GROUPCLASS_TV);
    pGroup->AddLight("1", 0.5, -0.6);
    pGroup->AddLight("2", 0.4, -0.5);
    pGroup->AddLight("3", 0.3, -0.4);
    pGroup->AddLight("4", 0.2, -0.3);
    pGroup->AddLight("5", 0.1, -0.2);
    pGroup->AddLight("6", 0.0, -0.1);
    pGroup->AddLight("7", -0.1, 0.0, 0.5);
    pGroup->AddLight("8", -0.2, 0.1, -0.2, lightName, lightModel);

    auto sceneTag = "jfdks";
    auto sceneName = "Savanna Sunset";
    auto sceneAppData = "HECxM_r12_d15";
    auto scene = std::make_shared<Scene>("", sceneTag, sceneName, sceneAppData);
    auto scenes = std::make_shared<SceneList>();
    scenes->push_back(scene);
    pGroup->SetScenes(scenes);

    JSONNode node;
    pGroup->Serialize(&node);
    auto g = std::static_pointer_cast<Group>(Group::DeserializeFromJson(&node));
    ASSERT_EQ("12", g->GetId());
    ASSERT_EQ(groupName, g->GetName());
    ASSERT_EQ(GROUPCLASS_TV, g->GetClassType());
    ASSERT_EQ(8, g->GetLights()->size());
    ASSERT_EQ("1", g->GetLights()->at(0)->GetId());
    ASSERT_EQ("2", g->GetLights()->at(1)->GetId());
    ASSERT_EQ("3", g->GetLights()->at(2)->GetId());
    ASSERT_EQ("4", g->GetLights()->at(3)->GetId());
    ASSERT_EQ("5", g->GetLights()->at(4)->GetId());
    ASSERT_EQ("6", g->GetLights()->at(5)->GetId());
    ASSERT_EQ("7", g->GetLights()->at(6)->GetId());
    ASSERT_EQ("8", g->GetLights()->at(7)->GetId());
    ASSERT_EQ(0.5, g->GetLights()->at(0)->GetPosition().GetX());
    ASSERT_EQ(0.4, g->GetLights()->at(1)->GetPosition().GetX());
    ASSERT_EQ(0.3, g->GetLights()->at(2)->GetPosition().GetX());
    ASSERT_EQ(0.2, g->GetLights()->at(3)->GetPosition().GetX());
    ASSERT_EQ(0.1, g->GetLights()->at(4)->GetPosition().GetX());
    ASSERT_EQ(0.0, g->GetLights()->at(5)->GetPosition().GetX());
    ASSERT_EQ(-0.1, g->GetLights()->at(6)->GetPosition().GetX());
    ASSERT_EQ(-0.2, g->GetLights()->at(7)->GetPosition().GetX());
    ASSERT_EQ(-0.6, g->GetLights()->at(0)->GetPosition().GetY());
    ASSERT_EQ(-0.5, g->GetLights()->at(1)->GetPosition().GetY());
    ASSERT_EQ(-0.4, g->GetLights()->at(2)->GetPosition().GetY());
    ASSERT_EQ(-0.3, g->GetLights()->at(3)->GetPosition().GetY());
    ASSERT_EQ(-0.2, g->GetLights()->at(4)->GetPosition().GetY());
    ASSERT_EQ(-0.1, g->GetLights()->at(5)->GetPosition().GetY());
    ASSERT_EQ(0.0, g->GetLights()->at(6)->GetPosition().GetY());
    ASSERT_EQ(0.1, g->GetLights()->at(7)->GetPosition().GetY());
    ASSERT_EQ(0.5, g->GetLights()->at(6)->GetPosition().GetZ());
    ASSERT_EQ(-0.2, g->GetLights()->at(7)->GetPosition().GetZ());
    ASSERT_EQ(lightName, g->GetLights()->at(7)->GetName());
    ASSERT_EQ(lightModel, g->GetLights()->at(7)->GetModel());

    ASSERT_EQ(sceneTag, g->GetScenes()->at(0)->GetTag());
    ASSERT_EQ(sceneName, g->GetScenes()->at(0)->GetName());
    ASSERT_EQ(sceneAppData, g->GetScenes()->at(0)->GetAppData());
    ASSERT_EQ("12", g->GetScenes()->at(0)->GetGroupId());
    ASSERT_EQ(15, g->GetScenes()->at(0)->GetDefaultSceneId());
}

TEST_F(TestGroup, AddLight) {
    auto name = "bedroom lamp";
    auto model = "LLC002";
    ASSERT_EQ(0, pGroup->GetLights()->size());
    pGroup->AddLight("1", 0.5, 0.6, 0.7, name, model);
    ASSERT_EQ(1, pGroup->GetLights()->size());
    ASSERT_EQ("1", pGroup->GetLights()->at(0)->GetId());
    ASSERT_EQ(0.5, pGroup->GetLights()->at(0)->GetPosition().GetX());
    ASSERT_EQ(0.6, pGroup->GetLights()->at(0)->GetPosition().GetY());
    ASSERT_EQ(0.7, pGroup->GetLights()->at(0)->GetPosition().GetZ());
    ASSERT_EQ(name, pGroup->GetLights()->at(0)->GetName());
    ASSERT_EQ(model, pGroup->GetLights()->at(0)->GetModel());
}

TEST_F(TestGroup, WhenLightExistsLightIsUpdated) {
    auto name = "nicelight";
    auto model = "LST001";

    pGroup->AddLight("1", 0.1, 0.2, 0.3);
    ASSERT_EQ("1", pGroup->GetLights()->at(0)->GetId());
    ASSERT_EQ(0.1, pGroup->GetLights()->at(0)->GetPosition().GetX());
    ASSERT_EQ(0.2, pGroup->GetLights()->at(0)->GetPosition().GetY());
    ASSERT_EQ("", pGroup->GetLights()->at(0)->GetName());
    ASSERT_EQ("", pGroup->GetLights()->at(0)->GetModel());

    pGroup->AddLight("1", 0.5, 0.6, 0.7, name, model);
    ASSERT_EQ("1", pGroup->GetLights()->at(0)->GetId());
    ASSERT_EQ(0.5, pGroup->GetLights()->at(0)->GetPosition().GetX());
    ASSERT_EQ(0.6, pGroup->GetLights()->at(0)->GetPosition().GetY());
    ASSERT_EQ(0.7, pGroup->GetLights()->at(0)->GetPosition().GetZ());
    ASSERT_EQ(name, pGroup->GetLights()->at(0)->GetName());
    ASSERT_EQ(model, pGroup->GetLights()->at(0)->GetModel());
}

TEST_F(TestGroup, XYAreClipped) {
    pGroup->AddLight("1", -2.3, -2.7);
    ASSERT_EQ("1", pGroup->GetLights()->at(0)->GetId());
    ASSERT_EQ(-1, pGroup->GetLights()->at(0)->GetPosition().GetX());
    ASSERT_EQ(-1, pGroup->GetLights()->at(0)->GetPosition().GetY());

    pGroup->AddLight("1", 2.3, 2.7);
    ASSERT_EQ("1", pGroup->GetLights()->at(0)->GetId());
    ASSERT_EQ(1, pGroup->GetLights()->at(0)->GetPosition().GetX());
    ASSERT_EQ(1, pGroup->GetLights()->at(0)->GetPosition().GetY());
}

TEST_F(TestGroup, Clone) {
    auto groupName = "My Entertainment group";
    auto lightName7 = "My light";
    auto lightModel7 = "LTW010";
    auto lightName8 = "Another light";
    auto lightModel8 = "LTW011";
    auto lightArchetype7 = "hue_go";
    auto lightArchetype8 = "hue_lightstrip";

    pGroup->SetId("12");
    pGroup->SetName(groupName);
    pGroup->SetClassType(GROUPCLASS_TV);
    pGroup->AddLight("1", 0.5, -0.6);
    pGroup->AddLight("2", 0.4, -0.5);
    pGroup->AddLight("3", 0.3, -0.4);
    pGroup->AddLight("4", 0.2, -0.3);
    pGroup->AddLight("5", 0.1, -0.2, 1.0);
    pGroup->AddLight("6", 0.0, -0.1, 0.1);
    pGroup->AddLight("7", -0.1, 0.0, -1.0, lightName7, lightModel7, lightArchetype7, true);
    pGroup->AddLight("8", -0.2, 0.1, -0.1, lightName8, lightModel8, lightArchetype8, false);

    auto g = std::shared_ptr<Group>(pGroup->Clone());
    pGroup->SetId("13");
    pGroup->SetName("bla bla");
    pGroup->SetClassType(GROUPCLASS_OTHER);
    pGroup->AddLight("9", 0, 0);
    pGroup->GetLights()->at(7)->SetName("Your light");
    pGroup->GetLights()->at(7)->SetModel("LTW011");

    ASSERT_EQ("12", g->GetId());
    ASSERT_EQ(groupName, g->GetName());
    ASSERT_EQ(GROUPCLASS_TV, g->GetClassType());
    ASSERT_EQ(8, g->GetLights()->size());
    ASSERT_EQ("2", g->GetLights()->at(1)->GetId());
    ASSERT_EQ("3", g->GetLights()->at(2)->GetId());
    ASSERT_EQ("4", g->GetLights()->at(3)->GetId());
    ASSERT_EQ("5", g->GetLights()->at(4)->GetId());
    ASSERT_EQ("6", g->GetLights()->at(5)->GetId());
    ASSERT_EQ("7", g->GetLights()->at(6)->GetId());
    ASSERT_EQ("8", g->GetLights()->at(7)->GetId());
    ASSERT_EQ(0.5, g->GetLights()->at(0)->GetPosition().GetX());
    ASSERT_EQ(0.4, g->GetLights()->at(1)->GetPosition().GetX());
    ASSERT_EQ(0.3, g->GetLights()->at(2)->GetPosition().GetX());
    ASSERT_EQ(0.2, g->GetLights()->at(3)->GetPosition().GetX());
    ASSERT_EQ(0.1, g->GetLights()->at(4)->GetPosition().GetX());
    ASSERT_EQ(0.0, g->GetLights()->at(5)->GetPosition().GetX());
    ASSERT_EQ(-0.1, g->GetLights()->at(6)->GetPosition().GetX());
    ASSERT_EQ(-0.2, g->GetLights()->at(7)->GetPosition().GetX());
    ASSERT_EQ(-0.6, g->GetLights()->at(0)->GetPosition().GetY());
    ASSERT_EQ(-0.5, g->GetLights()->at(1)->GetPosition().GetY());
    ASSERT_EQ(-0.4, g->GetLights()->at(2)->GetPosition().GetY());
    ASSERT_EQ(-0.3, g->GetLights()->at(3)->GetPosition().GetY());
    ASSERT_EQ(-0.2, g->GetLights()->at(4)->GetPosition().GetY());
    ASSERT_EQ(-0.1, g->GetLights()->at(5)->GetPosition().GetY());
    ASSERT_EQ(0.0, g->GetLights()->at(6)->GetPosition().GetY());
    ASSERT_EQ(0.1, g->GetLights()->at(7)->GetPosition().GetY());
    ASSERT_EQ(1.0, g->GetLights()->at(4)->GetPosition().GetZ());
    ASSERT_EQ(0.1, g->GetLights()->at(5)->GetPosition().GetZ());
    ASSERT_EQ(-1.0, g->GetLights()->at(6)->GetPosition().GetZ());
    ASSERT_EQ(-0.1, g->GetLights()->at(7)->GetPosition().GetZ());
    ASSERT_EQ(lightName7, g->GetLights()->at(6)->GetName());
    ASSERT_EQ(lightModel7, g->GetLights()->at(6)->GetModel());
    ASSERT_EQ(lightName8, g->GetLights()->at(7)->GetName());
    ASSERT_EQ(lightModel8, g->GetLights()->at(7)->GetModel());
    ASSERT_EQ(true, g->GetLights()->at(5)->Reachable());
    ASSERT_EQ(true, g->GetLights()->at(6)->Reachable());
    ASSERT_EQ(false, g->GetLights()->at(7)->Reachable());
}

TEST_F(TestGroup, OwnerName) {
    pGroup->SetOwnerName("testappname#testdevicename");
    ASSERT_EQ(pGroup->GetOwnerName(), "testappname#testdevicename");
    EXPECT_EQ(pGroup->GetFriendlyOwnerName(), "testappname (testdevicename)");
    EXPECT_EQ(pGroup->GetOwnerApplicationName(), "testappname");
    EXPECT_EQ(pGroup->GetOwnerDeviceName(), "testdevicename");

    pGroup->SetOwnerName("test12.,-char#dev&%@s");
    EXPECT_EQ(pGroup->GetOwnerName(), "test12.,-char#dev&%@s");
    EXPECT_EQ(pGroup->GetFriendlyOwnerName(), "test12.,-char (dev&%@s)");
    EXPECT_EQ(pGroup->GetOwnerApplicationName(), "test12.,-char");
    EXPECT_EQ(pGroup->GetOwnerDeviceName(), "dev&%@s");

    pGroup->SetOwnerName("test only app");
    EXPECT_EQ(pGroup->GetOwnerName(), "test only app");
    EXPECT_EQ(pGroup->GetFriendlyOwnerName(), "test only app");
    EXPECT_EQ(pGroup->GetOwnerApplicationName(), "test only app");
    EXPECT_EQ(pGroup->GetOwnerDeviceName(), "");

    pGroup->SetOwnerName("testStrange#");
    EXPECT_EQ(pGroup->GetOwnerName(), "testStrange#");
    EXPECT_EQ(pGroup->GetFriendlyOwnerName(), "testStrange");
    EXPECT_EQ(pGroup->GetOwnerApplicationName(), "testStrange");
    EXPECT_EQ(pGroup->GetOwnerDeviceName(), "");

    pGroup->SetOwnerName("test#wrong#format");
    EXPECT_EQ(pGroup->GetOwnerName(), "test#wrong#format");
    EXPECT_EQ(pGroup->GetFriendlyOwnerName(), "test#wrong#format");
    EXPECT_EQ(pGroup->GetOwnerApplicationName(), "test#wrong#format");
    EXPECT_EQ(pGroup->GetOwnerDeviceName(), "");
}

/******************************************************************************/
/*                                 END OF FILE                                */
/******************************************************************************/