#include <huestream/config/Config.h>
#include "gtest/gtest.h"
#include "huestream/common/language/DummyTranslator.h"

#include <string>
#include <memory>

using namespace huestream;
using namespace std;

static const std::string tag = "ID_PRESS_PUSH_LINK";
static const std::string error_tag = "PRESS_PUSH";
static const std::string error_answer = "Internal error";
static const std::string dutch_answer = " gevonden. Druk op de link knop van je Hue Bridge.";
static const std::string other_answer = " found. Press the push-link button on the Hue Bridge.";

class TestDummyTranslator : public testing::Test {
 protected:
    MessageTranslatorPtr _dummyTranslator;

    virtual void SetUp() {
        _dummyTranslator = std::make_shared<DummyTranslator>("nl");
    }

    virtual void TearDown() {
        _dummyTranslator.reset();
    }
};

TEST_F(TestDummyTranslator, Valid_Tag){
    ASSERT_EQ(other_answer, std::make_shared<DummyTranslator>("en")->Translate(tag));
    ASSERT_EQ(dutch_answer, std::make_shared<DummyTranslator>("nl")->Translate(tag));
    ASSERT_EQ(other_answer, std::make_shared<DummyTranslator>("by")->Translate(tag));
}

TEST_F(TestDummyTranslator, InValid_Tag){
    ASSERT_EQ(error_answer, std::make_shared<DummyTranslator>("en")->Translate(error_tag));
    ASSERT_EQ(error_answer, std::make_shared<DummyTranslator>("nl")->Translate(error_tag));
    ASSERT_EQ(error_answer, std::make_shared<DummyTranslator>("by")->Translate(error_tag));
}
/******************************************************************************/
/*                                 END OF FILE                                */
/******************************************************************************/
