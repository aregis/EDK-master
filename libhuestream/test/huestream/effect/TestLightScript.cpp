/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <memory>

#include "gtest/gtest.h"
#include "huestream/effect/lightscript/LightScript.h"
#include "huestream/config/ObjectBuilder.h"
#include "test/huestream/_mock/MockAnimationEffect.h"
#include "test/huestream/_mock/MockAction.h"


using namespace huestream;

using std::shared_ptr;
using testing::Return;
using testing::_;

class MockActionBuilder : public ObjectBuilderBase {

public:
    std::shared_ptr<Serializable> ConstructInstanceOf(std::string type) override;
};

std::shared_ptr<Serializable> MockActionBuilder::ConstructInstanceOf(std::string type) {
    if (type == Location::type) {
        return std::make_shared<Location>();
    }
    
    return std::make_shared<MockAction>();
}

class TestLightScript : public testing::Test {
protected:
    std::shared_ptr<LightScript> _lightscript;
    std::vector<std::shared_ptr<MockAnimationEffect>> _effects;

    virtual void SetUp() {
        _lightscript = std::make_shared<LightScript>();
        Serializable::SetObjectBuilder(std::make_shared<MockActionBuilder>());
    }

    virtual void TearDown() {
        for (auto effect : _effects) {
            effect.reset();
        }
        _effects.clear();
        Serializable::SetObjectBuilder(std::make_shared<ObjectBuilder>(nullptr));
    }

    virtual void AddAction(std::string name, unsigned int layer, long long int startPosition) {
        auto effect = std::make_shared<MockAnimationEffect>(name, layer);
        EXPECT_CALL(*effect, SetPlayer(_));
        EXPECT_CALL(*effect, Serialize(_));
        auto action = std::make_shared<MockAction>(name, layer, effect, startPosition);
        EXPECT_CALL(*action, GetTypeName());
        _lightscript->AddAction(action);
    }

    virtual bool HasAction(std::string name, unsigned int layer, long long int startPosition) {
        auto actions = _lightscript->GetActions();
        for (auto action : *actions) {
            if (action->GetName() == name && action->GetLayer() == layer && action->GetStartPosition() == startPosition)
                return true;
        }
        return false;
    }
};


TEST_F(TestLightScript, Serialize) {
    const std::string scriptName = "MyScript";
    _lightscript->SetName(scriptName);
    auto list = std::make_shared<LocationList>();
    list->push_back(std::make_shared<Location>(0, 1));
    list->push_back(std::make_shared<Location>(-0.5, 0.5));
    _lightscript->SetIdealSetup(list);
    _lightscript->SetLength(10000);
    AddAction("testEffect1", 0, 100);
    AddAction("testEffect2", 1, 0);
    AddAction("testEffect3", 1, 2000);

    JSONNode node;
    _lightscript->Serialize(&node);

    std::cout << node.write_formatted() << std::endl;

    _lightscript = std::make_shared<LightScript>();
    _lightscript->Deserialize(&node);

    ASSERT_EQ(_lightscript->GetName(), scriptName);
    ASSERT_EQ(_lightscript->GetIdealSetup()->size(), 2);
    ASSERT_EQ(_lightscript->GetLength(), 10000);
    ASSERT_EQ(_lightscript->GetActions()->size(), 3);
    ASSERT_TRUE(HasAction("testEffect1", 0, 100));
    ASSERT_TRUE(HasAction("testEffect2", 1, 0));
    ASSERT_TRUE(HasAction("testEffect3", 1, 2000));
}

TEST_F(TestLightScript, SetActionsOrdersList) {
    auto actionsIn = std::make_shared<ActionList>();
    actionsIn->push_back(std::make_shared<Action>("name1", 2, nullptr, 20, 40));
    actionsIn->push_back(std::make_shared<Action>("name2", 1, nullptr, 40, 50));
    actionsIn->push_back(std::make_shared<Action>("name3", 1, nullptr, 60, 60));
    actionsIn->push_back(std::make_shared<Action>("name4", 2, nullptr, 70, 80));
    _lightscript = std::make_shared<LightScript>();
    _lightscript->SetActions(actionsIn);

    ASSERT_EQ(_lightscript->GetActions()->size(), 4);
    ASSERT_EQ(_lightscript->GetActions()->at(0)->GetName(), "name2");
    ASSERT_EQ(_lightscript->GetActions()->at(1)->GetName(), "name3");
    ASSERT_EQ(_lightscript->GetActions()->at(2)->GetName(), "name1");
    ASSERT_EQ(_lightscript->GetActions()->at(3)->GetName(), "name4");

}
/******************************************************************************/
/*                                 END OF FILE                                */
/******************************************************************************/
