#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <thread>
#include "huestream/stream/DtlsTimerProvider.h"

namespace huestream {

class TestDefaultTimerProvider : public testing::Test {

};

TEST_F(TestDefaultTimerProvider, TestDelay) {
    auto _defaultTimerProvider = std::make_shared<DtlsTimerProvider>();

    _defaultTimerProvider->set_delay(3600, 0);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    EXPECT_EQ(-1, _defaultTimerProvider->get_delay());

    _defaultTimerProvider->set_delay(86400, 86400);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    EXPECT_EQ(0, _defaultTimerProvider->get_delay());

    _defaultTimerProvider->set_delay(0, 86400);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    EXPECT_EQ(1, _defaultTimerProvider->get_delay());

    _defaultTimerProvider->set_delay(0, 1);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    EXPECT_EQ(2, _defaultTimerProvider->get_delay());
}
}
