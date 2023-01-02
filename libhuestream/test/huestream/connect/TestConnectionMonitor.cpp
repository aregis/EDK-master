#include <huestream/connect/FeedbackMessage.h>
#include <huestream/connect/ConnectionMonitor.h>
#include <test/huestream/_mock/MockBridgeStateChecker.h>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

using ::testing::AtLeast;

namespace huestream {
    class TestConnectionMonitor : public testing::Test {
    public:
        virtual void SetUp() {
            _bridgeSettings = std::make_shared<BridgeSettings>();
            _bridge1 = std::make_shared<Bridge>(_bridgeSettings);
            _bridge1->SetId("bridge1");
            _bridge2 = std::make_shared<Bridge>(_bridgeSettings);
            _bridge2->SetId("bridge2");
            _mockBridgeStateChecker = std::shared_ptr<MockBridgeStateChecker>(new MockBridgeStateChecker());
            _connectionMonitor = std::make_shared<ConnectionMonitor>(std::static_pointer_cast<IBridgeStreamingChecker>(_mockBridgeStateChecker));
        }

        virtual void TearDown() {
            _connectionMonitor->Stop();
        }

        BridgePtr _bridge1;
        BridgePtr _bridge2;
        BridgeSettingsPtr _bridgeSettings;
        std::shared_ptr<MockBridgeStateChecker> _mockBridgeStateChecker;
        std::shared_ptr<ConnectionMonitor> _connectionMonitor;

    };

TEST_F(TestConnectionMonitor, Start_KicksCheckerPeriodically) {
    EXPECT_CALL(*_mockBridgeStateChecker, Check(_bridge1)).Times(AtLeast(1));
    _connectionMonitor->Start(_bridge1, 50);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
}

TEST_F(TestConnectionMonitor, Start_OverridesPreviousSettings) {
    EXPECT_CALL(*_mockBridgeStateChecker, Check(_bridge1)).Times(AtLeast(1));
    _connectionMonitor->Start(_bridge1, 50);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    EXPECT_CALL(*_mockBridgeStateChecker, Check(_bridge2)).Times(AtLeast(1));
    _connectionMonitor->Start(_bridge2, 50);
    EXPECT_CALL(*_mockBridgeStateChecker, Check(_bridge1)).Times(0);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
}

}  // namespace huestream
