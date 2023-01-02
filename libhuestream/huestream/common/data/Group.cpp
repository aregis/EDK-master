/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <huestream/common/data/Group.h>

#include <iostream>
#include <string>
#include <map>
#include <iostream>
#include <sstream>

namespace huestream {

    const std::map<GroupClass, std::string> Group::_classSerializeMap = {
        { GROUPCLASS_TV,      "tv" },
        { GROUPCLASS_FREE,    "free" },
        { GROUPCLASS_SCREEN,  "screen" },
        { GROUPCLASS_MUSIC,   "music" },
        { GROUPCLASS_3DSPACE, "3dspace" },
        { GROUPCLASS_OTHER,   "other" }
    };

    PROP_IMPL(Group, std::string, id, Id);
    PROP_IMPL(Group, std::string, groupedLightId, GroupedLightId);
    PROP_IMPL(Group, std::string, name, Name);
    PROP_IMPL(Group, GroupClass, classType, ClassType);
    PROP_IMPL(Group, LightListPtr, lights, Lights);
    PROP_IMPL(Group, LightListPtr, physicalLights, PhysicalLights);
    PROP_IMPL_BOOL(Group, bool, active, Active);
    PROP_IMPL(Group, std::string, owner, Owner);
    PROP_IMPL(Group, std::string, ownerName, OwnerName);
    PROP_IMPL(Group, GroupProxyNode, proxyNode, ProxyNode);
    PROP_IMPL(Group, SceneListPtr, scenes, Scenes);
    PROP_IMPL_BOOL(Group, bool, onState, OnState);
    PROP_IMPL(Group, double, brightnessState, BrightnessState);
    PROP_IMPL(Group, GroupChannelToPhysicalLightMapPtr, channelToPhysicalLightsMap, ChannelToPhysicalLightsMap);

    Group::Group() : _id(""), _groupedLightId(""), _name(""), _classType(GROUPCLASS_OTHER), _lights(std::make_shared<LightList>()), _physicalLights(std::make_shared<LightList>()), _active(false), _owner(""),
        _ownerName(""), _proxyNode({"","","","",true}), _scenes(std::make_shared<SceneList>()), _onState(true), _brightnessState(1.0), _channelToPhysicalLightsMap(std::make_shared<GroupChannelToPhysicalLightMap>()) {
    }

    Group::~Group() {
    }

    void Group::AddLight(std::string id, double x, double y, std::string name, std::string model, std::string archetype, bool reachable) {
        AddLight(id, x, y, 0.0, name, model, archetype, reachable);
    }

    void Group::AddLight(std::string id, double x, double y, double z, std::string name, std::string model, std::string archetype, bool reachable) {
        auto location = Location(Clip(x, -1, 1),
                                 Clip(y, -1, 1),
                                 Clip(z, -1, 1));

        Light newLight(id, location, name, model, archetype, reachable);
        AddLight(newLight);
    }

    void Group::AddLight(Light& light) {
      if (_lights == nullptr) {
        _lights = std::make_shared<LightList>();
      }

      for (auto &oldLight : *_lights) {
        if (oldLight->GetId() == light.GetId()) {
          oldLight->SetPosition(light.GetPosition());
          oldLight->SetName(light.GetName());
          oldLight->SetModel(light.GetModel());
          oldLight->SetReachable(light.Reachable());
          oldLight->SetColor(light.GetColor());
          return;
        }
      }

      _lights->push_back(std::shared_ptr<Light>(light.Clone()));
    }

    std::string Group::GetFriendlyOwnerName() const {
        auto appName = GetOwnerApplicationName();
        auto deviceName = GetOwnerDeviceName();

        std::ostringstream oss;
        oss << appName;
        if (!deviceName.empty()) {
            oss << " (" << deviceName << ")";
        }
        return oss.str();
    }

    std::string Group::GetOwnerApplicationName() const {
        auto splitOwner = Split(_ownerName, '#');
        if (splitOwner.size() == 2 || splitOwner.size() == 1) {
            return splitOwner.at(0);
        }
        return _ownerName;
    }

    std::string Group::GetOwnerDeviceName() const {
        auto splitOwner = Split(_ownerName, '#');
        if (splitOwner.size() == 2) {
            return splitOwner.at(1);
        }
        return std::string("");
    }

    LightList Group::GetChannelPhysicalLights(LightPtr channel)
    {
      LightList physicalLightList;

      auto physicalLightIdList = _channelToPhysicalLightsMap->find(channel->GetId());
      if (physicalLightIdList == _channelToPhysicalLightsMap->end())
      {
        return physicalLightList;
      }

      for (const auto& physicalLightId : physicalLightIdList->second)
      {
        for (auto physicalLight : *_physicalLights)
        {
          if (physicalLight->GetId() == physicalLightId)
          {
            physicalLightList.push_back(physicalLight);
            break;
          }
        }
      }

      return physicalLightList;
    }

    double Group::Clip(double value, double min, double max) const {
        if (value < min) {
            return min;
        }

        if (value > max) {
            return max;
        }

        return value;
    }

    void Group::Serialize(JSONNode *node) const {
        Serializable::Serialize(node);

        SerializeValue(node, AttributeId, _id);
        SerializeValue(node, AttributeGroupedLightId, _groupedLightId);
        SerializeValue(node, AttributeName, _name);
        SerializeClass(node);
        SerializeList(node, AttributeLights, _lights);
        SerializeList(node, AttributePhysicalLights, _physicalLights);
        SerializeList(node, AttributeScenes, _scenes);
    }

    void Group::Deserialize(JSONNode *node) {
        Serializable::Deserialize(node);
        DeserializeValue(node, AttributeId, &_id, "");
        DeserializeValue(node, AttributeGroupedLightId, &_groupedLightId, "");
        DeserializeValue(node, AttributeName, &_name, "");
        DeserializeClass(node);
        DeserializeList<LightListPtr, Light>(node, &_lights, AttributeLights);
        DeserializeList<LightListPtr, Light>(node, &_physicalLights, AttributePhysicalLights);
        DeserializeList<SceneListPtr, Scene>(node, &_scenes, AttributeScenes);
    }

    Group* Group::Clone() {
        auto g = new Group(*this);
        g->SetLights(clone_list(_lights));
        g->SetPhysicalLights(clone_list(_physicalLights));
        return g;
    }

    std::string Group::GetTypeName() const {
        return type;
    }

    void Group::SerializeClass(JSONNode *node) const {
        auto it = _classSerializeMap.find(_classType);
        if (it != _classSerializeMap.end()) {
            SerializeValue(node, AttributeClassType, it->second);
        } else {
            SerializeValue(node, AttributeClassType, "other");
        }
    }

    void Group::DeserializeClass(JSONNode *node) {
        std::string classString;
        DeserializeValue(node, AttributeClassType, &classString, "other");

        _classType = GROUPCLASS_OTHER;

        for (auto it = _classSerializeMap.begin(); it != _classSerializeMap.end(); ++it) {
            if (it->second == classString) {
                _classType = it->first;
                break;
            }
        }
    }

    std::vector<std::string> Group::Split(const std::string& s, char delimiter) const {
        std::vector<std::string> tokens;
        std::string token;
        std::istringstream tokenStream(s);
        while (std::getline(tokenStream, token, delimiter)) {
            tokens.push_back(token);
        }
        return tokens;
    }

}  // namespace huestream
