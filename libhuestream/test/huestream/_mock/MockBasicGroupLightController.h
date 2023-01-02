/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/
#ifndef LIBHUESTREAM_TEST_MOCKBASICGROUPCONTROLLER_H_
#define LIBHUESTREAM_TEST_MOCKBASICGROUPCONTROLLER_H_

#include <gmock/gmock.h>

#include <memory>

#include "huestream/connect/IBasicGroupLightController.h"

namespace huestream {

    class MockBasicGroupLightController : public IBasicGroupLightController {
    public:
        MockBasicGroupLightController() {}
        MOCK_METHOD1(SetActiveBridge, void(BridgePtr bridge));
        MOCK_METHOD1(SetOn, void(bool on));
        MOCK_METHOD1(SetBrightness, void(double brightness));
        MOCK_METHOD2(SetColor, void(double x, double y));
        MOCK_METHOD2(SetPreset, void(LightPreset preset, bool excludeLightsWhichAreOff));
        MOCK_METHOD4(SetPreset, void(double brightness, double x, double y, bool excludeLightsWhichAreOff));
        MOCK_METHOD1(SetScene, void(const std::string &sceneTag));
    };

    class MockWrapperBasicGroupLightController : public IBasicGroupLightController {
    public:
        explicit MockWrapperBasicGroupLightController(const std::shared_ptr<MockBasicGroupLightController>& mock)
           : _mock(mock) {}

        void SetActiveBridge(BridgePtr bridge) {
            _mock->SetActiveBridge(bridge);
        }

        void SetOn(bool on) {
            _mock->SetOn(on);
        }

        void SetBrightness(double brightness) {
            _mock->SetBrightness(brightness);
        }

        void SetColor(double x, double y) {
            _mock->SetColor(x, y);
        }

        void SetPreset(LightPreset preset, bool excludeLightsWhichAreOff) {
            _mock->SetPreset(preset, excludeLightsWhichAreOff);
        }

        void SetPreset(double brightness, double x, double y, bool excludeLightsWhichAreOff) {
            _mock->SetPreset(brightness, x, y, excludeLightsWhichAreOff);
        }

        void SetScene(const std::string& sceneTag) {
            _mock->SetScene(sceneTag);
        }

    private:
        std::shared_ptr<MockBasicGroupLightController> _mock;
    };

}// namespace huestream
#endif     // LIBHUESTREAM_TEST_MOCKBASICGROUPCONTROLLER_H_
