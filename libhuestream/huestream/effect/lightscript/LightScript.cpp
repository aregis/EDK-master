/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <huestream/effect/lightscript/LightScript.h>

#include <string>
#include <memory>

namespace huestream {

    std::string LightScript::type = "huestream.LightScript";
    std::string LightScript::AttributeVersionMajor = "VerionMajor";
    std::string LightScript::AttributeVersionMinor = "VersionMinor";
    int LightScript::majorVersion = 0;
    int LightScript::minorVersion = 0;

    PROP_IMPL(LightScript, std::string, name, Name);
    PROP_IMPL(LightScript, LocationListPtr, idealSetup, IdealSetup);
    PROP_IMPL(LightScript, int64_t, length, Length);
    PROP_IMPL_ON_UPDATE_CALL(LightScript, ActionListPtr, actions, Actions, UpdateActions);

    LightScript::LightScript() : LightScript("", 0) {}

    LightScript::LightScript(std::string name, int64_t length) : LightScript(name, length,
                                                                                   std::make_shared<LocationList>()) {}

    LightScript::LightScript(std::string name, int64_t length, LocationListPtr idealSetup) : _name(name),
                                                                                                   _idealSetup(
                                                                                                           idealSetup),
                                                                                                   _length(length),
                                                                                                   _actions(
                                                                                                           std::make_shared<ActionList>()) {}

    void LightScript::AddAction(ActionPtr newAction) {
        newAction->SetTimeProvider(_timeline);
        auto i = FindActionIndex(newAction);
        _actions->insert(_actions->begin() + i, newAction);
    }

    void LightScript::UpdateActions() {
        auto actionsCopy = std::make_shared<ActionList>(*_actions);
        _actions->clear();

        for (auto action : *actionsCopy) {
            AddAction(action);
        }
    }

    void LightScript::BindTimeline(TimelinePtr timeline) {
        _timeline = timeline;
        for (auto action : *_actions) {
            action->SetTimeProvider(timeline);
        }
    }

    TimelinePtr LightScript::GetTimeline() {
        return _timeline;
    }

    void LightScript::Finish() {
        for (auto action : *_actions) {
            action->Finish();
        }
    }

    int LightScript::FindActionIndex(const ActionPtr &newEffect) const {
        auto i = 0;

        for (auto action : *_actions) {
            auto newStartPosition = newEffect->GetStartPosition();
            auto startPosition = action->GetStartPosition();
            auto newLayer = newEffect->GetLayer();
            auto layer = action->GetLayer();
            if (newLayer < layer || (newStartPosition < startPosition && newLayer == layer)) {
                break;
            }
            i++;
        }

        return i;
    }

    std::string LightScript::GetTypeName() const {
        return type;
    }

    void LightScript::Serialize(JSONNode *node) const {
        Serializable::Serialize(node);
        SerializeValue(node, AttributeVersionMajor, majorVersion);
        SerializeValue(node, AttributeVersionMinor, minorVersion);
        SerializeValue(node, AttributeName, _name);

        if (!_idealSetup->empty())
            SerializeList(node, AttributeIdealSetup, _idealSetup);

        SerializeValue(node, AttributeLength, _length);

        SerializeLayers(node);
    }

    void LightScript::SerializeLayers(JSONNode *node) const {
        JSONNode layersNode(JSON_ARRAY);
        layersNode.set_name("layers");

        unsigned int currentLayerId = 0;
        auto actionList = std::make_shared<ActionList>();

        for (auto action : *_actions) {
            auto layerId = action->GetLayer();
            if (layerId != currentLayerId) {
                if (!actionList->empty()) {
                    JSONNode layer;
                    SerializeValue(&layer, "layer", currentLayerId);
                    SerializeList(&layer, "timeline", actionList);
                    layersNode.push_back(layer);
                    actionList->clear();
                }
                currentLayerId = layerId;
            }
            actionList->push_back(action);
        }
        if (!actionList->empty()) {
            JSONNode layer;
            SerializeValue(&layer, "layer", currentLayerId);
            SerializeList(&layer, "timeline", actionList);
            layersNode.push_back(layer);
        }

        node->push_back(layersNode);
    }

    void LightScript::Deserialize(JSONNode *node) {
        Serializable::Deserialize(node);

        int majorVersionDeserialized;
        DeserializeValue(node, AttributeVersionMajor, &majorVersionDeserialized, 0);
        if (majorVersionDeserialized > majorVersion)
            return;

        DeserializeValue(node, AttributeName, &_name, "");
        int length;
        DeserializeValue(node, AttributeLength, &length, 0);
        _length = length;
        DeserializeList<LocationListPtr, Location>(node, &_idealSetup, AttributeIdealSetup);

        DeserializeLayers(node);
    }

    void LightScript::DeserializeLayers(JSONNode *node) {
        auto currentLayer = 0;
        if (SerializerHelper::IsAttributeSet(node, "layers")) {
            auto layersNode = (*node)["layers"];
            for (auto layerIt = layersNode.begin(); layerIt != layersNode.end(); ++layerIt) {
                JSONNode layerNode = *layerIt;
                if (SerializerHelper::IsAttributeSet(&layerNode, "layer")) {
                    DeserializeValue(&layerNode, "layer", &currentLayer, 0);
                }
                if (SerializerHelper::IsAttributeSet(&layerNode, "timeline")) {
                    auto timelineNode = (layerNode)["timeline"];
                    for (auto actionIt = timelineNode.begin(); actionIt != timelineNode.end(); ++actionIt) {
                        JSONNode actionNode = *actionIt;
                        auto action = std::static_pointer_cast<Action>(DeserializeFromJson(&actionNode));
                        action->SetLayer(static_cast<unsigned int>(currentLayer));
                        AddAction(action);
                    }
                }
            }
        }
    }


}  // namespace huestream
