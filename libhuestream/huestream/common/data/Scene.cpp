/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <huestream/common/data/Scene.h>

#include <iostream>
#include <string>
#include <regex>

namespace huestream {

Scene::Scene(): Scene("", "", {}, "") {
}

Scene::Scene(const std::string& id, const std::string& tag, const std::string& name, const std::string& appData, const std::string& image)
    : _id(id), _tag(tag), _name(name), _appData(appData), _image(image), _lights(std::make_shared<LightList>()), _dynamic(false) {
}

Scene::~Scene() {
}

PROP_IMPL(Scene, std::string, id, Id);
PROP_IMPL(Scene, std::string, tag, Tag);
PROP_IMPL(Scene, std::string, name, Name);
PROP_IMPL(Scene, std::string, appData, AppData);
PROP_IMPL(Scene, std::string, image, Image);
PROP_IMPL(Scene, LightListPtr, lights, Lights);
PROP_IMPL_BOOL(Scene, bool, dynamic, Dynamic);

std::string Scene::GetGroupId() const {
    std::string id("");
    std::regex re(".*_r(\\d+)_d.*");
    std::smatch match;

    if (std::regex_search(_appData, match, re) && match.size() >= 2) {
        auto intId = std::stoi(match.str(1));
        id = std::to_string(intId);
    }

    return id;
}

int Scene::GetDefaultSceneId() const {
    int id = -1;
    std::regex re(".*_d(\\d+)");
    std::smatch match;

    if (std::regex_search(_appData, match, re) && match.size() >= 2) {
        id = std::stoi(match.str(1));
    }

    return id;
}

bool Scene::AllLightsAreDynamic() const
{
  for (auto lightIt = _lights->begin(); lightIt != _lights->end(); ++lightIt)
  {
    if (!(*lightIt)->Dynamic())
    {
      return false;
    }
  }

  return  true;
}

bool Scene::AtLeastOneLightIsDynamic() const
{
    for (auto lightIt = _lights->begin(); lightIt != _lights->end(); ++lightIt)
    {
        if ((*lightIt)->Dynamic())
        {
            return true;
        }
    }

    return false;
}

Scene* Scene::Clone() {
    auto s = new Scene();
    s->_id = _id;
    s->_tag = _tag;
    s->_name = _name;
    s->_appData = _appData;
    s->_image = _image;
    s->_lights = _lights;
    return  s;
}

void Scene::Serialize(JSONNode *node) const {
    Serializable::Serialize(node);
    SerializeValue(node, AttributeId, _id);
    SerializeValue(node, AttributeTag, _tag);
    SerializeValue(node, AttributeName, _name);
    SerializeValue(node, AttributeAppData, _appData);
    SerializeValue(node, AttributeImage, _image);
    // TODO serialize lights?
}

void Scene::Deserialize(JSONNode *node) {
    Serializable::Deserialize(node);
    DeserializeValue(node, AttributeId, &_id, "");
    DeserializeValue(node, AttributeTag, &_tag, "");
    DeserializeValue(node, AttributeName, &_name, "");
    DeserializeValue(node, AttributeAppData, &_appData, "");
    DeserializeValue(node, AttributeImage, &_image, "");
}

std::string Scene::GetTypeName() const {
    return type;
}

}  // namespace huestream
