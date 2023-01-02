/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#ifndef HUESTREAM_CONNECT_IBASICGROUPLIGHTCONTROLLER_H_
#define HUESTREAM_CONNECT_IBASICGROUPLIGHTCONTROLLER_H_

#include "huestream/common/data/Bridge.h"

#include <memory>
#include <string>

namespace huestream {

        class IBasicGroupLightController {
        public:
            /**
             type of light presets when not streaming
             */
            enum LightPreset {
                LIGHT_PRESET_RELAX,        ///< preset for relaxing
                LIGHT_PRESET_READ,         ///< preset for reading
                LIGHT_PRESET_CONCENTRATE,  ///< preset for concentrating
                LIGHT_PRESET_ENERGIZE      ///< preset for energizing
            };

            virtual ~IBasicGroupLightController() {}

            virtual void SetActiveBridge(BridgePtr bridge) = 0;

            virtual void SetOn(bool on) = 0;

            virtual void SetBrightness(double brightness) = 0;

            virtual void SetColor(double x, double y) = 0;

            virtual void SetPreset(LightPreset preset, bool excludeLightsWhichAreOff = false) = 0;

            virtual void SetPreset(double bri, double x, double y, bool excludeLightsWhichAreOff = false) = 0;

            virtual void SetScene(const std::string &sceneTag) = 0;

        protected:
        };

        typedef std::shared_ptr<IBasicGroupLightController> BasicGroupLightControllerPtr;

}  // namespace huestream

#endif  // HUESTREAM_CONNECT_IBASICGROUPLIGHTCONTROLLER_H_
