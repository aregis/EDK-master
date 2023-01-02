/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <huestream/connect/BasicGroupLightController.h>
#include <libjson/libjson.h>

#include <math.h>
#include <string>
#include <memory>
#include <sstream>

namespace huestream {

        BasicGroupLightController::BasicGroupLightController(BridgeHttpClientPtr http) : _http(http), _bridge(nullptr) {
        }

        std::map<BasicGroupLightController::LightPreset, std::tuple<double, double, double>> BasicGroupLightController::_presetSettingsMap = {
            { LIGHT_PRESET_RELAX,       std::make_tuple(0.567, 0.5018, 0.4152) },
            { LIGHT_PRESET_READ,        std::make_tuple(1.0,   0.4450, 0.4067) },
            { LIGHT_PRESET_CONCENTRATE, std::make_tuple(1.0,   0.3690, 0.3719) },
            { LIGHT_PRESET_ENERGIZE,    std::make_tuple(1.0,   0.3143, 0.3301) }
        };

        void BasicGroupLightController::SetActiveBridge(BridgePtr bridge) {
            std::unique_lock<std::mutex> lk(_mutex);
            _bridge = bridge;
        }

        void BasicGroupLightController::SetOn(bool on) {
        if (_bridge == nullptr || !_bridge->IsConnectable() || !_bridge->IsValidGroupSelected()) {
            return;
        }

        if (_bridge->IsSupportingClipV2()) {
            // Since idv1 of the group was removed from the bridge in clipv2, just loop through all lights and turn them on/off one by one
            GroupPtr group = _bridge->GetGroupById(_bridge->GetSelectedGroup());

            if (group == nullptr) {
                return;
            }

            LightListPtr physicalLightList = group->GetPhysicalLights();

            if (physicalLightList == nullptr) {
                return;
            }

            UpdateGroupOn(group, on);

            LightList lightList = *physicalLightList;

            for (auto lightIt = lightList.begin(); lightIt != lightList.end(); ++lightIt) {
                std::string url = _bridge->GetBaseUrl(true);
                if (url.empty()) {
                    return;
                }

                (*lightIt)->SetOn(on);

                url += "/light/";
                url += (*lightIt)->GetId();

                JSONNode body;

                JSONNode onNode;
                onNode.set_name("on");
                onNode.push_back(JSONNode("on", on));
                body.push_back(onNode);

                httpPut(url, body);
            }

            // Update other groups on state too in case they share a light
            GroupList groupList = *_bridge->GetGroups();
            for (auto groupIt = groupList.begin(); groupIt != groupList.end(); ++groupIt) {
                if (*groupIt != group) {
                    UpdateGroupLights(*groupIt, lightList, false);
                    }
                }
            }
            else {
                auto url = getBridgeUrl();
                if (url.empty()) {
                    return;
                }

                JSONNode actionNode;
                actionNode.push_back(JSONNode("on", on));

                httpPut(url, actionNode);
            }
        }

        void BasicGroupLightController::SetBrightness(double brightness) {
            if (_bridge == nullptr || !_bridge->IsConnectable() || !_bridge->IsValidGroupSelected()) {
                return;
            }

            if (_bridge->IsSupportingClipV2()) {
                // Since idv1 of the group was removed from the bridge in clipv2, just loop through all lights and turn them on/off one by one
                GroupPtr group = _bridge->GetGroupById(_bridge->GetSelectedGroup());

                if (group == nullptr) {
                    return;
                }

                LightListPtr physicalLightList = group->GetPhysicalLights();

                if (physicalLightList == nullptr) {
                    return;
                }

                UpdateGroupBrightness(group, brightness);

                LightList lightList = *physicalLightList;

                for (auto lightIt = lightList.begin(); lightIt != lightList.end(); ++lightIt) {
                    std::string url = _bridge->GetBaseUrl(true);
                    if (url.empty()) {
                        return;
                    }

                    (*lightIt)->SetBrightness(brightness * 100.0);

                    url += "/light/";
                    url += (*lightIt)->GetId();

                    JSONNode body;

                    JSONNode dimmingNode;
                    dimmingNode.set_name("dimming");
                    dimmingNode.push_back(JSONNode("brightness", brightness * 100));
                    body.push_back(dimmingNode);

                    httpPut(url, body);
                }

                // Update other groups brightness state too in case they share a light
                GroupList groupList = *_bridge->GetGroups();
                for (auto groupIt = groupList.begin(); groupIt != groupList.end(); ++groupIt) {
                    if (*groupIt != group) {
                        UpdateGroupLights(*groupIt, lightList, false);
                    }
                }
            }
            else {
                auto url = getBridgeUrl();
                if (url.empty()) {
                    return;
                }

                auto bri = static_cast<int>(round(brightness * 254));
                JSONNode actionNode;
                actionNode.push_back(JSONNode("bri", bri));

                httpPut(url, actionNode);
            }
        }

        void BasicGroupLightController::SetColor(double x, double y) {
            if (_bridge == nullptr || !_bridge->IsConnectable() || !_bridge->IsValidGroupSelected()) {
                return;
            }

            if (_bridge->IsSupportingClipV2()) {
                // Since idv1 of the group was removed from the bridge in clipv2, just loop through all lights and change their color one by one
                GroupPtr group = _bridge->GetGroupById(_bridge->GetSelectedGroup());

                if (group == nullptr) {
                    return;
                }

                LightListPtr physicalLightList = group->GetPhysicalLights();

                if (physicalLightList == nullptr) {
                    return;
                }

                UpdateGroupColor(group, x, y, group->GetBrightnessState(), true);

                LightList lightList = *physicalLightList;

                for (auto lightIt = lightList.begin(); lightIt != lightList.end(); ++lightIt) {
                    std::string url = _bridge->GetBaseUrl(true);
                    if (url.empty()) {
                        return;
                    }

                    url += "/light/";
                    url += (*lightIt)->GetId();

                    JSONNode body;
                    JSONNode color;
                    JSONNode xy;

                    xy.set_name("xy");
                    xy.push_back(JSONNode("x", x));
                    xy.push_back(JSONNode("y", y));

                    color.set_name("color");
                    color.push_back(xy);

                    body.push_back(color);

                    httpPut(url, body);
                }

                // Update other groups brightness state too in case they share a light
                GroupList groupList = *_bridge->GetGroups();
                for (auto groupIt = groupList.begin(); groupIt != groupList.end(); ++groupIt) {
                    if (*groupIt != group) {
                        UpdateGroupLights(*groupIt, lightList, false);
                    }
                }
            }
            else {
                auto url = getBridgeUrl();
                if (url.empty()) {
                    return;
                }

                JSONNode xyNode(JSON_ARRAY);
                xyNode.push_back(JSONNode("", x));
                xyNode.push_back(JSONNode("", y));
                xyNode.set_name("xy");
                JSONNode actionNode;
                actionNode.push_back(xyNode);

                httpPut(url, actionNode);
            }
        }

        void BasicGroupLightController::SetPreset(LightPreset preset, bool excludeLightsWhichAreOff) {
            auto i = _presetSettingsMap.find(preset);
            if (i == _presetSettingsMap.end()) {
                return;
            }
            auto settings = i->second;
            auto bri = std::get<0>(settings);
            auto x = std::get<1>(settings);
            auto y = std::get<2>(settings);

            SetPreset(bri, x, y, excludeLightsWhichAreOff);
        }

        void BasicGroupLightController::SetPreset(double brightness, double x, double y, bool excludeLightsWhichAreOff) {
            if (_bridge == nullptr || !_bridge->IsConnectable() || !_bridge->IsValidGroupSelected()) {
                return;
            }

            if (_bridge->IsSupportingClipV2()) {
                // Since idv1 of the group was removed from the bridge in clipv2, just loop through all lights and change their color one by one
                GroupPtr group = _bridge->GetGroupById(_bridge->GetSelectedGroup());

                if (group == nullptr) {
                    return;
                }

                LightListPtr physicalLightList = group->GetPhysicalLights();

                if (physicalLightList == nullptr) {
                    return;
                }

                UpdateGroupColor(group, x, y, brightness, true);

                LightList lightList = *physicalLightList;

                for (auto lightIt = lightList.begin(); lightIt != lightList.end(); ++lightIt) {
                    std::string url = _bridge->GetBaseUrl(true);
                    if (url.empty()) {
                        return;
                    }

                    url += "/light/";
                    url += (*lightIt)->GetId();

                    JSONNode body;

                    JSONNode color;
                    JSONNode xy;
                    xy.set_name("xy");
                    xy.push_back(JSONNode("x", x));
                    xy.push_back(JSONNode("y", y));
                    color.set_name("color");
                    color.push_back(xy);
                    body.push_back(color);

                    JSONNode dimmingNode;
                    dimmingNode.set_name("dimming");
                    dimmingNode.push_back(JSONNode("brightness", brightness * 100));
                    body.push_back(dimmingNode);

                    if (!excludeLightsWhichAreOff) {
                        JSONNode onNode;
                        onNode.set_name("on");
                        onNode.push_back(JSONNode("on", true));
                        body.push_back(onNode);
                    }

                    httpPut(url, body);
                }

                // Update other groups on and brightness state too in case they share a light
                GroupList groupList = *_bridge->GetGroups();
                for (auto groupIt = groupList.begin(); groupIt != groupList.end(); ++groupIt) {
                    if (*groupIt != group) {
                        UpdateGroupLights(*groupIt, lightList, false);
                    }
                }
            }
            else {
                auto url = getBridgeUrl();
                if (url.empty()) {
                    return;
                }

                auto bri = static_cast<int>(round(brightness * 254));
                JSONNode actionNode;
                if (!excludeLightsWhichAreOff) {
                    actionNode.push_back(JSONNode("on", true));
                }
                actionNode.push_back(JSONNode("bri", bri));
                JSONNode xyNode(JSON_ARRAY);
                xyNode.push_back(JSONNode("", x));
                xyNode.push_back(JSONNode("", y));
                xyNode.set_name("xy");
                actionNode.push_back(xyNode);

                httpPut(url, actionNode);
            }
        }

        void BasicGroupLightController::SetScene(const std::string &sceneTag) {
            if (_bridge->IsSupportingClipV2()) {
                return;
            }

            auto url = getBridgeUrl();
            if (url.empty()) {
                return;
            }

            JSONNode actionNode;
            actionNode.push_back(JSONNode("scene", sceneTag));

            httpPut(url, actionNode);
        }

        std::string BasicGroupLightController::getBridgeUrl() {
            std::unique_lock<std::mutex> lk(_mutex);
            std::string url;
            if (_bridge != nullptr && _bridge->IsConnectable() && _bridge->IsValidGroupSelected()) {
                url = _bridge->GetSelectedGroupActionUrl();
            }
            return url;
        }

        void BasicGroupLightController::httpPut(const std::string & url, const JSONNode & actionNode) {
            auto body = actionNode.write();
            _http->ExecuteHttpRequest(_bridge, HTTP_REQUEST_PUT, url, body, {});
        }

        void BasicGroupLightController::UpdateGroupBrightness(GroupPtr group, double brightness) {
            if (group == nullptr) {
                return;
            }

            LightListPtr physicalLightList = group->GetPhysicalLights();

            if (physicalLightList == nullptr) {
                return;
            }

            group->SetBrightnessState(brightness);

            LightList lightList = *physicalLightList;

            for (auto lightIt = lightList.begin(); lightIt != lightList.end(); ++lightIt) {
                (*lightIt)->SetBrightness(brightness * 100.0);
            }
        }

        void BasicGroupLightController::UpdateGroupOn(GroupPtr group, bool on) {
            if (group == nullptr) {
                return;
            }

            LightListPtr physicalLightList = group->GetPhysicalLights();

            if (physicalLightList == nullptr) {
                return;
            }

            group->SetOnState(on);

            LightList lightList = *physicalLightList;

            for (auto lightIt = lightList.begin(); lightIt != lightList.end(); ++lightIt) {
                (*lightIt)->SetOn(on);
            }
        }

        void BasicGroupLightController::UpdateGroupColor(GroupPtr group, double x, double y, double brightness, bool aUpdateLightBrightness) {
            if (group == nullptr) {
                return;
            }

            LightListPtr physicalLightList = group->GetPhysicalLights();

            if (physicalLightList == nullptr) {
                return;
            }

            LightList lightList = *physicalLightList;

            double xy[2] = { x, y };

            for (auto lightIt = lightList.begin(); lightIt != lightList.end(); ++lightIt) {
                (*lightIt)->SetColor({xy, brightness, 1.0});
                if (aUpdateLightBrightness) {
                    (*lightIt)->SetBrightness({ brightness * 100.0 });
                }
            }

            group->SetBrightnessState(brightness);
        }

        void BasicGroupLightController::UpdateGroupLights(huestream::GroupPtr group, const huestream::LightList& fromLightList, bool aUpdateColor) {
            if (group == nullptr) {
                return;
            }

            LightListPtr physicalLightList = group->GetPhysicalLights();

            if (physicalLightList == nullptr) {
                return;
            }

            LightList lightsList = *physicalLightList;

            double brightness = 0.0;
            bool isOn = false;
            uint32_t numReachableLights = 0;

            for (auto fromLightIt = fromLightList.begin(); fromLightIt != fromLightList.end(); ++fromLightIt) {
                auto groupLightIt = std::find_if(std::begin(lightsList), std::end(lightsList), [&](const LightPtr groupLight) {
                    return groupLight->GetId() == (*fromLightIt)->GetId();
                });

                if (groupLightIt != std::end(lightsList)) {
                    LightPtr groupLight = *groupLightIt;

                    if (aUpdateColor) {
                        const Color& color = (*fromLightIt)->GetColor();
                        if (!(color.GetR() == 0.0 && color.GetG() == 0.0 && color.GetB() == 0.0)) {
                            groupLight->SetColor(color);

                            double xy[2] = { 0.0, 0.0 };
                            double Y = 0.0;
                            color.GetYxy(Y, xy[0], xy[1]);
                            groupLight->SetBrightness(Y);
                        }
                    }
                    else {
                        groupLight->SetBrightness((*fromLightIt)->GetBrightness());
                    }

                    groupLight->SetOn((*fromLightIt)->On());

                    if (groupLight->Reachable()) {
                        brightness += groupLight->GetBrightness() / 100.0;
                        numReachableLights++;
                        isOn = isOn || groupLight->On();
                    }
                }
            }

            // Update group brightness too
            if (numReachableLights > 0)
            {
              group->SetOnState(isOn);
              group->SetBrightnessState(brightness / numReachableLights);
            }
        }

}  // namespace huestream
