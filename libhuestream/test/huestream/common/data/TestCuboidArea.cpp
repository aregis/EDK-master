#include "gtest/gtest.h"
#include "huestream/common/data/CuboidArea.h"
#include "huestream/config/ObjectBuilder.h"

#include <string>

using namespace huestream;
using namespace std;

class TestCuboidArea : public testing::Test {
protected:

    shared_ptr<CuboidArea> _frontBottomLeft;
    shared_ptr<CuboidArea> _backHalf;

    virtual void SetUp() {
        Serializable::SetObjectBuilder(std::make_shared<ObjectBuilder>(nullptr));
        _frontBottomLeft = make_shared<CuboidArea>(Location(-1, 1, 0), Location(0, 0, -1), "FrontBottomLeft");
        _backHalf = make_shared<CuboidArea>(Location(-1, 0, 1), Location(1, -1, -1), "BackHalf");
    }

    virtual void TearDown() {
        Serializable::SetObjectBuilder(nullptr);
    }
};

TEST_F(TestCuboidArea, SimpleArea) {
    EXPECT_DOUBLE_EQ(_frontBottomLeft->GetTopFrontLeft().GetX(), -1);
    EXPECT_DOUBLE_EQ(_frontBottomLeft->GetTopFrontLeft().GetY(), 1);
    EXPECT_DOUBLE_EQ(_frontBottomLeft->GetTopFrontLeft().GetZ(), 0);
    EXPECT_DOUBLE_EQ(_frontBottomLeft->GetBottomBackRight().GetX(), 0);
    EXPECT_DOUBLE_EQ(_frontBottomLeft->GetBottomBackRight().GetY(), 0);
    EXPECT_DOUBLE_EQ(_frontBottomLeft->GetBottomBackRight().GetZ(), -1);

    ASSERT_TRUE(_frontBottomLeft->isInArea(Location(-0.5, 0.5, -0.5)));
    ASSERT_FALSE(_frontBottomLeft->isInArea(Location(0.5, 0.5, -0.5)));
    ASSERT_FALSE(_frontBottomLeft->isInArea(Location(-0.5, -0.5, -0.5)));
    ASSERT_FALSE(_frontBottomLeft->isInArea(Location(-0.5, 0.5, 0.5)));
    ASSERT_FALSE(_frontBottomLeft->isInArea(Location(0.5, -0.5, 0.5)));

    _frontBottomLeft->SetBottomBackRight(Location(0, -1, -1));
    ASSERT_TRUE(_frontBottomLeft->isInArea(Location(-0.5, -0.5, -0.5)));

    ASSERT_TRUE(_backHalf->isInArea(Location(0, -0.5, 0)));
    ASSERT_TRUE(_backHalf->isInArea(Location(0.5, -0.5, -0.5)));
    ASSERT_FALSE(_backHalf->isInArea(Location(0, 0.5, 0)));

    _backHalf->SetTopFrontLeft(Location(-1, 1, 1));
    ASSERT_TRUE(_backHalf->isInArea(Location(0, 0.5, 0)));
}

TEST_F(TestCuboidArea, Invert) {
    _frontBottomLeft->SetInverted(true);
    ASSERT_FALSE(_frontBottomLeft->isInArea(Location(-0.5, 0.5, -0.5)));
    ASSERT_TRUE(_frontBottomLeft->isInArea(Location(0.5, 0.5, -0.5)));
    ASSERT_TRUE(_frontBottomLeft->isInArea(Location(-0.5, -0.5, -0.5)));
    ASSERT_TRUE(_frontBottomLeft->isInArea(Location(-0.5, 0.5, 0.5)));
    ASSERT_TRUE(_frontBottomLeft->isInArea(Location(0.5, -0.5, 0.5)));
}

TEST_F(TestCuboidArea, Serialize) {
    JSONNode node;
    _frontBottomLeft->Serialize(&node);
    std::string jc = node.write_formatted();

    std::cout << jc << std::endl;

    auto a = std::make_shared<CuboidArea>();
    std::string type = "huestream.CuboidArea";
    ASSERT_EQ(a->GetTypeName(), type);
    ASSERT_FALSE(a->isInArea(Location(-0.5, 0.5, -0.5)));
    a->Deserialize(&node);
    ASSERT_TRUE(a->isInArea(Location(-0.5, 0.5, -0.5)));
    ASSERT_FALSE(a->isInArea(Location(0.5, 0.5, -0.5)));
}

/******************************************************************************/
/*                                 END OF FILE                                */
/******************************************************************************/
