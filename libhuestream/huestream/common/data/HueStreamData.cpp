/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <huestream/common/data/HueStreamData.h>

#include <memory>
#include <regex>

namespace huestream {

PROP_IMPL(HueStreamData, int, version, Version);
PROP_IMPL(HueStreamData, BridgeListPtr, bridges, Bridges);

HueStreamData::HueStreamData(BridgeSettingsPtr bridgeSettings)
    : _version(0), _bridges(std::make_shared<BridgeList>()), _bridgeSettings(bridgeSettings) {
    _bridges->push_back(std::make_shared<Bridge>(bridgeSettings));
}

void HueStreamData::Clear() {
    _bridges = std::make_shared<BridgeList>();
    _bridges->push_back(std::make_shared<Bridge>(_bridgeSettings));
}

BridgePtr HueStreamData::GetActiveBridge() const {
    if (_bridges->empty()) {
        _bridges->push_back(std::make_shared<Bridge>(_bridgeSettings));
    }
    return _bridges->back();
}

void HueStreamData::SetActiveBridge(BridgePtr bridge) {
    if (!GetActiveBridge()->HasEverBeenAuthorizedForStreaming()) {
        _bridges->pop_back();
    }
    RemoveBridge(bridge);
    _bridges->push_back(bridge);
}

void HueStreamData::ClearActiveBridge() {
    if (!_bridges->empty()) {
        _bridges->pop_back();
    }
    _bridges->push_back(std::make_shared<Bridge>(_bridgeSettings));
}

bool HueStreamData::RemoveBridge(BridgePtr bridge) {
    auto removed = false;
    if (!_bridges->empty()) {
        auto index = std::find_if(_bridges->begin(), _bridges->end(),
            [bridge](const BridgePtr &arg) { return arg->GetId() == bridge->GetId(); });

        if (index != _bridges->end()) {
            _bridges->erase(index);
            removed = true;
        }
    }
    return removed;
}

bool HueStreamData::RediscoverKnownBridge(BridgeListPtr discoveredBridges) {
    BridgePtr activeBridge = nullptr;
    for (auto rit = _bridges->rbegin(); rit != _bridges->rend(); ++rit) {
        auto knownBridge = *rit;
        if (knownBridge->IsAuthorizedForStreaming()) {
            for (auto it = discoveredBridges->begin(); it != discoveredBridges->end(); ++it) {
                auto discoveredBridge = *it;
                if (discoveredBridge->GetId() == knownBridge->GetId()) {
                    knownBridge->SetIpAddress(discoveredBridge->GetIpAddress());
                    knownBridge->SetIsValidIp(true);
                    if (activeBridge == nullptr) {
                      activeBridge = knownBridge;
                    }
                    break;
                }
            }
        }
    }

    if (activeBridge != nullptr) {
      SetActiveBridge(activeBridge);
      return true;
    }

    return false;
}

BridgeListPtr HueStreamData::GetAllKnownBridges() {
    auto knownBridges = std::make_shared<BridgeList>();
    for (auto rit = _bridges->rbegin(); rit != _bridges->rend(); ++rit) {
        auto bridge = *rit;
        if (bridge->HasEverBeenAuthorizedForStreaming()) {
            knownBridges->push_back(bridge);
        }
    }
    return knownBridges;
}

void HueStreamData::Serialize(JSONNode *node) const {
    Serializable::Serialize(node);
    SerializeValue(node, AttributeVersion, _version);
    SerializeList(node, AttributeBridges, _bridges);
}

void HueStreamData::Deserialize(JSONNode *node) {
    Serializable::Deserialize(node);
    DeserializeValue(node, AttributeVersion, &_version, 0);
    DeserializeList<BridgeListPtr, Bridge>(node, &_bridges, AttributeBridges);
}

std::string HueStreamData::GetTypeName() const {
    return type;
}

}  // namespace huestream
