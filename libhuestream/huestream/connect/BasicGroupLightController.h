/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#ifndef HUESTREAM_CONNECT_BASICGROUPLIGHTCONTROLLER_H_
#define HUESTREAM_CONNECT_BASICGROUPLIGHTCONTROLLER_H_

#include "huestream/common/http/IBridgeHttpClient.h"
#include "huestream/connect/IBasicGroupLightController.h"

#include <memory>
#include <string>
#include <map>
#include <tuple>

namespace huestream {

        class BasicGroupLightController : public IBasicGroupLightController {
        public:
            explicit BasicGroupLightController(BridgeHttpClientPtr http);

            virtual void SetActiveBridge(BridgePtr bridge) override;

            virtual void SetOn(bool on) override;

            virtual void SetBrightness(double bri) override;

            virtual void SetColor(double x, double y) override;

            virtual void SetPreset(LightPreset preset, bool excludeLightsWhichAreOff = false) override;

            virtual void SetPreset(double bri, double x, double y, bool excludeLightsWhichAreOff = false) override;

            virtual void SetScene(const std::string &sceneId) override;

        protected:
            static std::map<LightPreset, std::tuple<double, double, double>> _presetSettingsMap;

            BridgeHttpClientPtr _http;
            BridgePtr _bridge;
            std::mutex _mutex;

            std::string getBridgeUrl();
            void httpPut(const std::string &url, const JSONNode &actionNode);

            void UpdateGroupBrightness(GroupPtr group, double brightness);
            void UpdateGroupOn(GroupPtr group, bool on);
            void UpdateGroupColor(GroupPtr group, double x, double y, double brightness, bool aUpdateLightBrightness = false);
            void UpdateGroupLights(huestream::GroupPtr group, const huestream::LightList& fromLightList, bool aUpdateColor = true);
        };

}  // namespace huestream

#endif  // HUESTREAM_CONNECT_BASICGROUPLIGHTCONTROLLER_H_
