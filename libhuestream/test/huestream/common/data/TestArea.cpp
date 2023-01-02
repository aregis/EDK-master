#include "gtest/gtest.h"
#include "huestream/common/data/Area.h"
#include "huestream/config/ObjectBuilder.h"

#include <string>

using namespace huestream;
using namespace std;

class TestArea : public testing::Test {
protected:

    shared_ptr<Area> _frontLeft;
    shared_ptr<Area> _backHalf;

    virtual void SetUp() {
        Serializable::SetObjectBuilder(std::make_shared<ObjectBuilder>(nullptr));
        _frontLeft = make_shared<Area>(Location(-1, 1), Location(0, 0), "FrontLeft");
        _backHalf = make_shared<Area>(Location(-1, 0), Location(1, -1), "BackHalf");
    }

    virtual void TearDown() {
        Serializable::SetObjectBuilder(nullptr);
    }
};

TEST_F(TestArea, SimpleArea) {
    EXPECT_DOUBLE_EQ(_frontLeft->GetTopLeft().GetX(), -1);
    EXPECT_DOUBLE_EQ(_frontLeft->GetTopLeft().GetY(), 1);
    EXPECT_DOUBLE_EQ(_frontLeft->GetBottomRight().GetX(), 0);
    EXPECT_DOUBLE_EQ(_frontLeft->GetBottomRight().GetY(), 0);

    ASSERT_TRUE(_frontLeft->isInArea(Location(-0.5, 0.5)));
    ASSERT_FALSE(_frontLeft->isInArea(Location(0.5, 0.5)));
    ASSERT_FALSE(_frontLeft->isInArea(Location(0.5, -0.5)));
    ASSERT_FALSE(_frontLeft->isInArea(Location(-0.5, -0.5)));

    _frontLeft->SetBottomRight(Location(0, -1));
    ASSERT_TRUE(_frontLeft->isInArea(Location(-0.5, -0.5)));

    ASSERT_TRUE(_backHalf->isInArea(Location(0, -0.5)));
    ASSERT_FALSE(_backHalf->isInArea(Location(0, 0.5)));

    _backHalf->SetTopLeft(Location(-1, 1));
    ASSERT_TRUE(_backHalf->isInArea(Location(0, 0.5)));
}

TEST_F(TestArea, Invert) {
    _frontLeft->SetInverted(true);
    ASSERT_FALSE(_frontLeft->isInArea(Location(-0.5, 0.5)));
    ASSERT_TRUE(_frontLeft->isInArea(Location(0.5, 0.5)));
    ASSERT_TRUE(_frontLeft->isInArea(Location(0.5, -0.5)));
    ASSERT_TRUE(_frontLeft->isInArea(Location(-0.5, -0.5)));
}

TEST_F(TestArea, Serialize) {
    JSONNode node;
    _frontLeft->Serialize(&node);
    std::string jc = node.write_formatted();

    std::cout << jc << std::endl;

    auto a = std::make_shared<Area>();
    std::string type = "huestream.Area";
    ASSERT_EQ(a->GetTypeName(), type);
    ASSERT_FALSE(a->isInArea(Location(-0.5, 0.5)));
    a->Deserialize(&node);
    ASSERT_TRUE(a->isInArea(Location(-0.5, 0.5)));
    ASSERT_FALSE(a->isInArea(Location(0.5, 0.5)));
}

/******************************************************************************/
/*                                 END OF FILE                                */
/******************************************************************************/
