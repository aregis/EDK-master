/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <algorithm>
#include <memory>
#include <string>
#include <regex>

#include "huestream/connect/FullConfigRetriever.h"
#include "huestream/common/http/IBridgeHttpClient.h"
#include "huestream/common/data/ApiVersion.h"
#include "support/network/http/HttpRequestError.h"
#include "support/network/http/IHttpResponse.h"
#include "support/network/http/HttpRequest.h"

namespace huestream {

#define CLIP_ERROR_TYPE_UNAUTHORIZED_USER 1

    ConfigRetriever::ConfigRetriever(const BridgeHttpClientPtr http, bool useForcedActivation, ConfigType configType) :
    _configType(configType), _http(http), _useForcedActivation(useForcedActivation), _busy(false) {
    }

        bool ConfigRetriever::Execute(BridgePtr bridge, RetrieveCallbackHandler cb, FeedbackHandler fh) {
            std::unique_lock<std::mutex> lk(_mutex);
            if (_busy)
            {
                return false;
            }
            _busy = true;

            _bridge = bridge;
            // With ClipV2 this config retriever only retrieve small config, see BridgeConfigRetriever for the full config
            _configType = _bridge->IsSupportingClipV2() ? ConfigType::Small : _configType;
            _cb = cb;
            RetrieveConfig();
            return true;
        }

    void ConfigRetriever::RetrieveConfig() {
        const auto configUrl = _configType == ConfigType::Small ? _bridge->GetSmallConfigUrl() : _bridge->GetBaseUrl();

        std::weak_ptr<ConfigRetriever> lifetime = shared_from_this();

        _http->ExecuteHttpRequest(_bridge, HTTP_REQUEST_GET, configUrl, "", [lifetime, this](const support::HttpRequestError& error, const support::IHttpResponse& response) {
            std::shared_ptr<ConfigRetriever> ref = lifetime.lock();
            if (ref == nullptr) {
                return;
            }

                        _bridge->SetLastHttpErrorCode(error.get_code());

            if (error.get_code() == support::HttpRequestError::HTTP_REQUEST_ERROR_CODE_SUCCESS) {
                _response = response.get_body();
                ParseResponseAndExecuteCallback();
            } else if (error.get_code() != support::HttpRequestError::HTTP_REQUEST_ERROR_CODE_CANCELED) {
                _bridge->SetIsValidIp(false);
                Finish(OPERATION_FAILED);
            }
        });
    }

    void ConfigRetriever::ParseResponseAndExecuteCallback() {
        if (!ParseToJson()) {
            Finish(OPERATION_FAILED);
            return;
        }

        if (!CheckForNoErrorsInResponse()) {
            Finish(OPERATION_FAILED);
            return;
        }

        if (!ParseConfig()) {
            Finish(OPERATION_FAILED);
            return;
        }

        if (_configType == ConfigType::Full) {
            ParseGroups();
            ParseCapabilities();
        }

        Finish(OPERATION_SUCCESS);
    }

    bool ConfigRetriever::ParseToJson() {
        JSONNode n = libjson::parse(_response);
        if (n.type() == JSON_NULL) {
            _bridge->SetIsValidIp(false);
            return false;
        }
        _root = n;
        return true;
    }

    bool ConfigRetriever::CheckForNoErrorsInResponse() const {
        bool hasError = false;
        bool hasUnauthorizedError = false;

        if (_root.type() == JSON_ARRAY) {
            for (auto it = _root.begin(); it != _root.end(); ++it) {
                auto entry = it->find("error");
                if (entry != it->end()) {
                    hasError = true;
                    auto i = entry->find("type");
                    if (i != entry->end()) {
                        if (i->as_int() == CLIP_ERROR_TYPE_UNAUTHORIZED_USER) {
                            hasUnauthorizedError = true;
                        }
                    }
                    break;
                }
            }
        }

        if (hasError) {
            if (hasUnauthorizedError) {
                _bridge->SetIsAuthorized(false);
                _bridge->SetIsValidIp(true);
            } else {
                _bridge->SetIsValidIp(false);
            }
            return false;
        }

        // At this point we should be authorized and have a valid ip, so just make sure those flags are right.
        if (_configType == ConfigType::Full)
        {
          _bridge->SetIsAuthorized(true);
        }

        _bridge->SetIsValidIp(true);

        return true;
    }

    bool ConfigRetriever::ParseConfig() const {
        if (_configType == ConfigType::Full && !SerializerHelper::IsAttributeSet(&_root, "config")) {
            _bridge->SetIsValidIp(false);
            return false;
        }

        JSONNode config = _configType == ConfigType::Full ? (_root)["config"] : _root;
        std::string name = "";
        Serializable::DeserializeValue(&config, "name", &name, "");
        _bridge->SetName(name);

        std::string modelid = "";
        Serializable::DeserializeValue(&config, "modelid", &modelid, "");
        _bridge->SetModelId(modelid);

        std::string apiversion = "";
        Serializable::DeserializeValue(&config, "apiversion", &apiversion, "");
        _bridge->SetApiversion(apiversion);

                std::string swversion = "";
                Serializable::DeserializeValue(&config, "swversion", &swversion, "");
                _bridge->SetSwversion(swversion);

        // ModelId is only introduced in api version 1.8.0, but older versions are always BSB001
        ApiVersion thisVersion(apiversion);
        ApiVersion modelIdSupportVersion("1.8.0");
        if (_bridge->GetModelId().empty() && thisVersion.IsValid() && thisVersion < modelIdSupportVersion) {
            _bridge->SetModelId("BSB001");
        }

        std::string bridgeid = "";
        Serializable::DeserializeValue(&config, "bridgeid", &bridgeid, "");
        _bridge->SetId(bridgeid);

        if (_configType == ConfigType::Full && !SerializerHelper::IsAttributeSet(&_root, "groups")) {
            _bridge->SetIsValidIp(false);
            return false;
        }

        if (_configType == ConfigType::Full && !SerializerHelper::IsAttributeSet(&_root, "lights")) {
            _bridge->SetIsValidIp(false);
            return false;
        }

        _bridge->SetIsValidIp(true);
        return true;
    }

    void ConfigRetriever::ParseGroups() {
        JSONNode groups = (_root)["groups"];
        auto parsedGroups = std::make_shared<GroupList>();
        for (auto jsonGroup = groups.begin(); jsonGroup != groups.end(); ++jsonGroup) {
            if (GroupIsEntertainment(*jsonGroup)) {
                auto group = ParseEntertainmentGroup(*jsonGroup);
                parsedGroups->push_back(group);
            }
        }
        _bridge->SetGroups(parsedGroups);
    }

    bool ConfigRetriever::GroupIsEntertainment(const JSONNode &j) {
        return (SerializerHelper::IsAttributeSet(&j, "type") && j["type"].as_string() == "Entertainment");
    }

    std::shared_ptr<Group> ConfigRetriever::ParseEntertainmentGroup(const JSONNode &node) {
        auto groupId = node.name();

        auto group = std::make_shared<Group>();
        group->SetId(groupId);

        std::string name = "";
        Serializable::DeserializeValue(&node, "name", &name, "");
        group->SetName(name);

        ParseClass(node, group);

        ParseLightsAndLocations(node, group);

        ParseStream(node, group);

        ParseGroupState(node, group);

        ParseScenes(node, group);

        return group;
    }

    void ConfigRetriever::ParseClass(const JSONNode &node, GroupPtr group) {
        if (SerializerHelper::IsAttributeSet(&node, "class")) {
                        if (node["class"].as_string() == "TV") {
                                group->SetClassType(GROUPCLASS_TV);
                        }
                        else if (node["class"].as_string() == "Free") {
                                group->SetClassType(GROUPCLASS_FREE);
            } else {
                group->SetClassType(GROUPCLASS_OTHER);
            }
        }
    }

    void ConfigRetriever::ParseLightsAndLocations(const JSONNode &node, GroupPtr group) const {
        auto avgBri = 0.0;
        auto numReachableLights = 0;
        LightListPtr physicalLightList = std::make_shared<LightList>();

        if (SerializerHelper::IsAttributeSet(&node, "locations")) {
            auto locations = node["locations"];
            for (auto i = locations.begin(); i != locations.end(); ++i) {
                auto lightId = i->name();
                auto coordinates = *i;
                auto light = GetLightInfo(i->name());

                if (light.GetModel() == "LCX001" || light.GetModel() == "LCX002" || light.GetModel() == "LCX003") {
                    physicalLightList->push_back(std::shared_ptr<Light>(light.Clone()));
                    //Add fixed lights for PXLS
                    light.SetId("100");
                    light.SetPosition({ -0.4, 0.8, -0.4 });
                    group->AddLight(light);
                    light.SetId("101");
                    light.SetPosition({ -0.4, 0.8, 0.0 });
                    group->AddLight(light);
                    light.SetId("102");
                    light.SetPosition({ -0.4, 0.8, 0.4 });
                    group->AddLight(light);
                    light.SetId("103");
                    light.SetPosition({ 0.0, 0.8, 0.4 });
                    group->AddLight(light);
                    light.SetId("104");
                    light.SetPosition({ 0.4, 0.8, 0.4 });
                    group->AddLight(light);
                    light.SetId("105");
                    light.SetPosition({ 0.4, 0.8, 0.0 });
                    group->AddLight(light);
                    light.SetId("106");
                    light.SetPosition({ 0.4, 0.8, -0.4 });
                    group->AddLight(light);
                }
                else {
                    auto location = Location(Clip(coordinates[0].as_float(), -1, 1),
                    Clip(coordinates[1].as_float(), -1, 1),
                    Clip(coordinates[2].as_float(), -1, 1));
                    light.SetPosition(location);
                    group->AddLight(light);

                    physicalLightList->push_back(std::shared_ptr<Light>(light.Clone()));
                }

                if (light.Reachable()) {
                    avgBri += light.GetBrightness();
                    numReachableLights++;
                }
            }
        }
        if (numReachableLights > 0) {
            avgBri /= numReachableLights;
        }
        group->SetBrightnessState(avgBri / 254);
        group->SetPhysicalLights(physicalLightList);
    }

    void ConfigRetriever::ParseStream(const JSONNode &node, GroupPtr group) {
        if (SerializerHelper::IsAttributeSet(&node, "stream")) {
            auto stream = node["stream"];
            ParseStreamActive(stream, group);
            ParseStreamProxy(stream, group);
        }
    }

    void ConfigRetriever::ParseStreamActive(const JSONNode &node, GroupPtr group) {
        if (SerializerHelper::IsAttributeSet(&node, "active") &&
            SerializerHelper::IsAttributeSet(&node, "owner")) {
            auto active = node["active"].as_bool();
            group->SetActive(active);
            if (active) {
                auto owner = node["owner"].as_string();
                group->SetOwner(owner);
                group->SetOwnerName(GetOwnerName(owner));
            }
        }
    }

    void ConfigRetriever::ParseStreamProxy(const JSONNode &node, GroupPtr group) {
        if (SerializerHelper::IsAttributeSet(&node, "proxymode") &&
            SerializerHelper::IsAttributeSet(&node, "proxynode")) {
            GroupProxyNode proxyNode;
            proxyNode.uri = node["proxynode"].as_string();
            proxyNode.mode = node["proxymode"].as_string();

            std::regex re("/lights/(\\d+)");
            std::smatch match;
            if (std::regex_search(proxyNode.uri, match, re) && match.size() == 2) {
                auto light = GetLightInfo(match.str(1));
                proxyNode.name = light.GetName();
                proxyNode.model = light.GetModel();
                proxyNode.isReachable = light.Reachable();
            } else {
                proxyNode.name = _bridge->GetName();
                proxyNode.model = _bridge->GetModelId();
                proxyNode.isReachable = true;
            }

            group->SetProxyNode(proxyNode);
        }
    }

    void ConfigRetriever::ParseGroupState(const JSONNode &node, GroupPtr group) {
        if (SerializerHelper::IsAttributeSet(&node, "state")) {
            auto state = node["state"];
            if (SerializerHelper::IsAttributeSet(&state, "any_on")) {
                auto anyOn = state["any_on"].as_bool();
                group->SetOnState(anyOn);
            }
        }
    }

    std::string ConfigRetriever::GetOwnerName(const std::string &userName) {
        std::string name = "";
        JSONNode config = (_root)["config"];
        if (SerializerHelper::IsAttributeSet(&config, "whitelist")) {
            JSONNode whitelist = (config)["whitelist"];
            if (SerializerHelper::IsAttributeSet(&whitelist, userName)) {
                JSONNode user = (whitelist)[userName];
                Serializable::DeserializeValue(&user, "name", &name, "");
            }
        }
        return name;
    }

    LightInfo ConfigRetriever::GetLightInfo(const std::string &id) const {
        std::string name = "";
        std::string model = "";
        auto reachable = true;
        auto on = true;
        auto bri = 0.0;
        double xy[2] = { 0.0, 0.0 };

        JSONNode lights = (_root)["lights"];
        if (SerializerHelper::IsAttributeSet(&lights, id)) {
            JSONNode light = (lights)[id];
            Serializable::DeserializeValue(&light, "name", &name, "");
            Serializable::DeserializeValue(&light, "modelid", &model, "");
            if (SerializerHelper::IsAttributeSet(&light, "state")) {
                JSONNode state = (light)["state"];
                Serializable::DeserializeValue(&state, "reachable", &reachable, true);
                Serializable::DeserializeValue(&state, "on", &on, true);
                if (on) {
                    Serializable::DeserializeValue(&state, "bri", &bri, 1.0);

                    if (SerializerHelper::IsAttributeSet(&state, "xy")) {
                      JSONNode xyJSON = state["xy"];
                      xy[0] = xyJSON[0].as_float();
                      xy[1] = xyJSON[1].as_float();
                    }
                }
            }
        }
        LightInfo lightInfo;
        lightInfo.SetId(id);
        lightInfo.SetName(name);
        lightInfo.SetModel(model);
        lightInfo.SetReachable(reachable);
        lightInfo.SetBrightness(bri);
        lightInfo.SetColor({ xy, bri });
        return lightInfo;
    }

    void ConfigRetriever::ParseScenes(const JSONNode &node, GroupPtr group) {
        if (!SerializerHelper::IsAttributeSet(&_root, "scenes")) {
            return;
        }
        JSONNode scenes = (_root)["scenes"];
        auto parsedScenes = std::make_shared<SceneList>();
        for (auto jsonScene = scenes.begin(); jsonScene != scenes.end(); ++jsonScene) {
            if (SceneIsNotRecyclable(*jsonScene)) {
                auto scene = ParseScene(*jsonScene);
                if (scene->GetGroupId() == group->GetId()) {
                    parsedScenes->push_back(scene);
                }
            }
        }
        group->SetScenes(parsedScenes);
    }

    bool ConfigRetriever::SceneIsNotRecyclable(const JSONNode &j) {
        return (SerializerHelper::IsAttributeSet(&j, "recycle") && j["recycle"].as_bool() == false);
    }

    ScenePtr ConfigRetriever::ParseScene(const JSONNode &node) {
        std::string name("");
        std::string appData("");
        Serializable::DeserializeValue(&node, "name", &name, "");
        if (SerializerHelper::IsAttributeSet(&node, "appdata")) {
            JSONNode appDataNode = (node)["appdata"];
            Serializable::DeserializeValue(&appDataNode, "data", &appData, "");
        }

        return std::make_shared<Scene>("", node.name(), name, appData);
    }

    void ConfigRetriever::ParseCapabilities() const {
        if (!SerializerHelper::IsAttributeSet(&_root, "capabilities")) {
            return;
        }
        JSONNode capabilities = (_root)["capabilities"];
        if (!SerializerHelper::IsAttributeSet(&capabilities, "streaming")) {
            return;
        }
        JSONNode streaming = (capabilities)["streaming"];

        int total;
        Serializable::DeserializeValue(&streaming, "total", &total, 0);
        _bridge->SetMaxNoStreamingSessions(total);

        if (!_bridge->IsStreaming()) {
            auto selectedGroupCanBeOvertaken = _bridge->IsValidGroupSelected() && _bridge->GetGroup()->Active() && _useForcedActivation;
            int available;
            Serializable::DeserializeValue(&streaming, "available", &available, -1);
            auto busy = (available == 0) && !selectedGroupCanBeOvertaken;
            _bridge->SetIsBusy(busy);
        }
    }

    void ConfigRetriever::Finish(OperationResult result) {
        if (result == OPERATION_FAILED && _bridge->IsValidGroupSelected() &&
            _bridge->GetGroup()->Active() && _bridge->GetGroup()->GetOwner() == _bridge->GetUser()) {
            _bridge->GetGroup()->SetActive(false);
            _bridge->GetGroup()->SetOwner("");
        }

        _cb(result, _bridge);
        _busy = false;
    }

    double ConfigRetriever::Clip(double value, double min, double max) const {
        if (value < min)
        {
          return min;
        }

        if (value > max)
        {
          return max;
        }

        return value;
    }

}  // namespace huestream
