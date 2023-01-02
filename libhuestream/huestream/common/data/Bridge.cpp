/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <huestream/common/data/Bridge.h>
#include <huestream/common/data/ApiVersion.h>
#include <huestream/config/Config.h>

#include <map>
#include <memory>
#include <regex>
#include <sstream>
#include <string>
#include <utility>

namespace huestream {

#define NO_SELECTED_GROUP "___NO_SELECTED_GROUP___"

std::map<BridgeStatus, std::string> Bridge::statusTagMap = {
    {BRIDGE_EMPTY,                  "ID_NO_BRIDGE_FOUND"},
    {BRIDGE_NOT_FOUND,              "ID_NO_BRIDGE_FOUND"},
    {BRIDGE_INVALID_MODEL,          "ID_INVALID_MODEL"},
    {BRIDGE_NOT_CONNECTABLE,        "ID_BRIDGE_NOT_FOUND"},
    {BRIDGE_NOT_CONNECTED,          "ID_BRIDGE_NOT_FOUND"},
    {BRIDGE_INVALID_VERSION,        "ID_INVALID_VERSION"},
    {BRIDGE_INVALID_CLIENTKEY,      "BRIDGE_INVALID_CLIENTKEY"},
    {BRIDGE_NO_GROUP_AVAILABLE,     "ID_NO_GROUP_AVAILABLE"},
    {BRIDGE_INVALID_GROUP_SELECTED, "ID_SELECT_GROUP"},
    {BRIDGE_BUSY,                   "ID_BRIDGE_BUSY"},
    {BRIDGE_READY,                  "BRIDGE_READY"},
    {BRIDGE_STREAMING,              "BRIDGE_STREAMING" }
};

PROP_IMPL(Bridge, BridgeSettingsPtr, bridgeSettings, BridgeSettings);
PROP_IMPL(Bridge, std::string, name, Name);
PROP_IMPL(Bridge, std::string, modelId, ModelId);
PROP_IMPL(Bridge, std::string, apiversion, Apiversion);
PROP_IMPL(Bridge, std::string, swversion, Swversion);
PROP_IMPL(Bridge, std::string, id, Id);
PROP_IMPL(Bridge, std::string, ipAddress, IpAddress);
PROP_IMPL(Bridge, std::string, sslPort, SslPort);
PROP_IMPL_BOOL(Bridge, bool, isValidIp, IsValidIp);
PROP_IMPL_BOOL(Bridge, bool, isAuthorized, IsAuthorized);
PROP_IMPL_BOOL(Bridge, bool, isBusy, IsBusy);
PROP_IMPL(Bridge, std::string, clientKey, ClientKey);
PROP_IMPL_ON_UPDATE_CALL(Bridge, std::string, user, User, OnUserChange);
PROP_IMPL(Bridge, GroupListPtr, groups, Groups);
PROP_IMPL(Bridge, std::string, selectedGroup, SelectedGroup);
PROP_IMPL(Bridge, int, maxNoStreamingSessions, MaxNoStreamingSessions);
PROP_IMPL(Bridge, std::string, certificate, Certificate);
PROP_IMPL(Bridge, std::string, appId, AppId);
PROP_IMPL(Bridge, support::HttpRequestError::ErrorCode, lastHttpErrorCode, LastHttpErrorCode);
PROP_IMPL(Bridge, int32_t, lastHttpStatusCode, LastHttpStatusCode);
PROP_IMPL(Bridge, ZoneListPtr, zones, Zones);

Bridge::Bridge(BridgeSettingsPtr bridgeSettings)
    : Bridge("", "", false, bridgeSettings) {
}

Bridge::Bridge(std::string id, std::string ip, bool ipValid, BridgeSettingsPtr bridgeSettings) :
    _bridgeSettings(bridgeSettings),
    _apiversion(""),
        _swversion(""),
    _id(id),
    _ipAddress(ip),
    _isValidIp(ipValid),
    _isAuthorized(false),
    _isBusy(false),
    _groups(std::make_shared<GroupList>()),
    _zones(std::make_shared<ZoneList>()),
    _selectedGroup(NO_SELECTED_GROUP),
    _maxNoStreamingSessions(0),
    _lastHttpErrorCode(support::HttpRequestError::HTTP_REQUEST_ERROR_CODE_SUCCESS){
}

void Bridge::Clear() {
    SetName("");
    SetIsAuthorized(false);
    SetIsValidIp(false);
    SetIsBusy(false);
    SetUser("");
        SetAppId("");
    SetId("");
    SetIpAddress("");
    SetApiversion("");
    SetModelId("");
    SetClientKey("");
    SetSelectedGroup("");
    SetGroups(std::make_shared<GroupList>());
    SetZones(std::make_shared<ZoneList>());
    SetLastHttpErrorCode(support::HttpRequestError::HTTP_REQUEST_ERROR_CODE_SUCCESS);
}

BridgeStatus Bridge::GetStatus() const {
    if (IsEmpty()) {
        return BRIDGE_EMPTY;
    }
    if (!IsFound()) {
        return BRIDGE_NOT_FOUND;
    }
    if (!IsValidModelId()) {
        return BRIDGE_INVALID_MODEL;
    }
    if (!IsConnectable()) {
        return BRIDGE_NOT_CONNECTABLE;
    }
    if (!IsConnected()) {
        return BRIDGE_NOT_CONNECTED;
    }
    if (!IsValidApiVersion()) {
        return BRIDGE_INVALID_VERSION;
    }
    if (!IsValidClientKey()) {
        return BRIDGE_INVALID_CLIENTKEY;
    }
    if (GetGroups()->size() == 0) {
        return BRIDGE_NO_GROUP_AVAILABLE;
    }
    if (!IsValidGroupSelected()) {
        return BRIDGE_INVALID_GROUP_SELECTED;
    }
    if (IsBusy()) {
        return BRIDGE_BUSY;
    }

    if (GetGroup()->Active() && GetGroup()->GetOwner() == GetAppId()) {
        return BRIDGE_STREAMING;
    }

    return BRIDGE_READY;
}

std::string Bridge::GetStatusTag() const {
    auto it = statusTagMap.find(GetStatus());
    if (it != statusTagMap.end())
        return it->second;
    return "UNKNOWN";
}

std::string Bridge::GetUserStatus(MessageTranslatorPtr translator) const {
    return translator->Translate(GetStatusTag());
}

bool Bridge::IsEmpty() const {
    return _ipAddress.empty();
}

bool Bridge::IsFound() const {
    return !IsEmpty() && !_id.empty();
}

bool Bridge::IsValidModelId() const {
    std::regex re("(?:BSB|HSE)(\\d+)");
    std::smatch match;
    if (!std::regex_search(_modelId, match, re) || match.size() != 2) {
        return false;
    }
    auto version = std::stoi(match.str(1));

    return version >= _bridgeSettings->GetSupportedModel();
}

bool Bridge::IsConnectable() const {
    return IsFound() &&
        !_user.empty() &&
        IsAuthorized() &&
        IsValidModelId();
}

bool Bridge::IsConnected() const {
    return IsConnectable() && IsValidIp();
}

bool Bridge::IsValidApiVersion() const {
    ApiVersion thisVersion(_apiversion);
    ApiVersion minVersion(_bridgeSettings->GetSupportedApiVersionMajor(),
        _bridgeSettings->GetSupportedApiVersionMinor(),
        _bridgeSettings->GetSupportedApiVersionBuild());

    return thisVersion.IsValid() && thisVersion >= minVersion;
}

bool Bridge::IsValidClientKey() const {
    return _clientKey.length() == 32;
}

bool Bridge::IsSupportingClipV2() const {
    if (_swversion.empty())
    {
        return false;
    }

    long int swversion = std::strtol(_swversion.c_str(), nullptr, 10);

    return swversion >= _bridgeSettings->GetSupportedClipV2SwVersion() && (_modelId.empty() || IsValidModelId());
}

bool Bridge::IsGroupSelected() const {
    return _selectedGroup != NO_SELECTED_GROUP;
}

bool Bridge::IsValidGroupSelected() const {
    return (GetGroup() != nullptr);
}

bool Bridge::IsProxyNodeUnreachable() const {
    auto group = GetGroup();
    if (group == nullptr) {
        return false;
    }

    return !group->GetProxyNode().isReachable;
}

bool Bridge::IsReadyToStream() const {
    return (GetStatus() == BRIDGE_READY);
}

bool Bridge::IsStreaming() const {
    return (GetStatus() == BRIDGE_STREAMING);
}

bool Bridge::HasEverBeenAuthorizedForStreaming() const {
    return IsFound() &&
        IsValidModelId() &&
        !_user.empty() &&
        IsValidClientKey();
}

bool Bridge::IsAuthorizedForStreaming() const {
    return HasEverBeenAuthorizedForStreaming() && IsAuthorized();
}

bool Bridge::SelectGroupIfOnlyOneOption() {
    // Always make a copy of the pointer because the original one could be changed from another thread.
    GroupListPtr groupList = _groups;
    if (groupList->size() == 1) {
        _selectedGroup = groupList->at(0)->GetId();
        return true;
    }
    return false;
}

bool Bridge::SelectGroup(std::string id) {
    // Always make a copy of the pointer because the original one could be changed from another thread.
    GroupListPtr groupList = _groups;
    for (auto i = groupList->begin(); i != groupList->end(); ++i) {
        if ((*i)->GetId() == id) {
            SetSelectedGroup(id);
            return true;
        }
    }
    return false;
}

GroupPtr Bridge::GetGroup() const {
    return GetGroupById(_selectedGroup);
}

LightListPtr Bridge::GetGroupLights() const {
    auto group = GetGroup();
    if (group == nullptr) {
        return std::make_shared<LightList>();
    }
    return group->GetLights();
}

GroupPtr Bridge::GetGroupById(const std::string& id) const {
    // Always make a copy of the pointer because the original one could be changed from another thread.
    GroupListPtr groupList = _groups;
    for (auto i = groupList->begin(); i != groupList->end(); ++i) {
        if ((*i)->GetId() == id) {
            return (*i);
        }
    }
    return nullptr;
}

ZonePtr Bridge::GetZoneById(const std::string& id) const
{
    // Always make a copy of the pointer because the original one could be changed from another thread.
    ZoneListPtr zoneList = _zones;
    for (auto i = zoneList->begin(); i != zoneList->end(); ++i)
    {
        if ((*i)->GetId() == id)
        {
            return (*i);
        }
    }
    return nullptr;
}

void Bridge::DeleteGroup(std::string id) {
    // Always make a copy of the pointer because the original one could be changed from another thread.
    GroupListPtr groupList = _groups;
    for (auto i = groupList->begin(); i != groupList->end(); ++i) {
        if ((*i)->GetId() == id) {
            groupList->erase(i);
            break;
        }
    }
}

GroupListPtr Bridge::GetGroupsOwnedByOtherClient() const {
    auto groups = std::make_shared<GroupList>();
    // Always make a copy of the pointer because the original one could be changed from another thread.
    GroupListPtr groupList = _groups;
    for (auto i = groupList->begin(); i != groupList->end(); ++i) {
        if ((*i)->Active() && (*i)->GetOwner() != _appId) {
            groups->push_back(*i);
        }
    }
    return groups;
}

int Bridge::GetCurrentNoStreamingSessions() const {
    int sessions = 0;
    // Always make a copy of the pointer because the original one could be changed from another thread.
    GroupListPtr groupList = _groups;
    for (auto i = groupList->begin(); i != groupList->end(); ++i) {
        if ((*i)->Active()) {
            sessions++;
        }
    }
    return sessions;
}

void Bridge::GetStreamApiRootUrl(std::ostringstream& aOSS, bool useClipV2, bool eventing) const {
      aOSS << GetUrl().c_str();

        if (useClipV2 && IsSupportingClipV2()) {
            if (eventing)   {
                aOSS << "/eventstream/clip/v2";
            }
            else {
                aOSS << "/clip/v2/resource";
            }
        }   else {
            aOSS << "/api/";
        }
}

std::string Bridge::GetApiRootUrl(bool useClipV2, bool eventing) const {
    std::ostringstream oss;
    GetStreamApiRootUrl(oss, useClipV2, eventing);
    return oss.str();
}

std::string Bridge::GetBaseUrl(bool useClipV2, bool eventing) const {
    std::ostringstream oss;
    GetStreamApiRootUrl(oss, useClipV2, eventing);
    if (!useClipV2 && !_user.empty()) {
                oss << _user;
                oss << "/";
    }

    return oss.str();
}

std::string Bridge::GetUrl() const
{
    std::ostringstream oss;

    auto protocol = "https";
    oss << protocol << "://" << _ipAddress;

    if (!_sslPort.empty())
    {
        oss << ":" << _sslPort;
    }

    return oss.str();
}

std::string Bridge::GetSmallConfigUrl() const {
    std::ostringstream oss;
    GetStreamApiRootUrl(oss);

    oss << "config/";

    return oss.str();
}

std::string Bridge::GetSelectedGroupUrl() const {
    std::ostringstream oss;

        if (IsSupportingClipV2()) {
                GetStreamApiRootUrl(oss, true, false);
                oss << "/" << "entertainment_configuration/" << _selectedGroup;
        }
        else
        {
                GetStreamApiRootUrl(oss);
                oss << _user << "/" << "groups/" << _selectedGroup;
        }

    return oss.str();
}

std::string Bridge::GetSelectedGroupActionUrl() const {
    std::ostringstream oss;

        if (IsSupportingClipV2())
        {
            // TODO there's no group associated with an entertainment area on the bridge with clipv2. Replace with zone when it will be available.
        }
        else {
                GetStreamApiRootUrl(oss);
                oss << _user << "/" << "groups/" << _selectedGroup << "/" << "action";
        }

    return oss.str();
}

std::string Bridge::GetAppIdUrl() const {
    std::string url = GetUrl();
    url += "/auth/v1";

    return url;
}

void Bridge::Serialize(JSONNode *node) const {
    SerializeBase(node);
    SerializeList(node, AttributeGroups, _groups);
    SerializeList(node, AttributeZones, _zones);
}

void Bridge::Deserialize(JSONNode *node) {
    DeserializeBase(node);
    DeserializeList<GroupListPtr, Group>(node, &_groups, AttributeGroups);
    DeserializeList<ZoneListPtr, Zone>(node, &_zones, AttributeZones);
}

std::string Bridge::GetTypeName() const {
    return type;
}

std::string Bridge::SerializeCompact() const {
    JSONNode n;
    SerializeBase(&n);
    return n.write();
}

void Bridge::DeserializeCompact(std::string jsonString) {
    auto n = libjson::parse(jsonString);
    DeserializeBase(&n);
}

void Bridge::SerializeBase(JSONNode *node) const {
    Serializable::Serialize(node);

    SerializeValue(node, AttributeName, _name);
    SerializeValue(node, AttributeModelId, _modelId);
    SerializeValue(node, AttributeApiversion, _apiversion);
    SerializeValue(node, AttributeSwversion, _swversion);
    SerializeValue(node, AttributeId, _id);
    SerializeValue(node, AttributeIpAddress, _ipAddress);
    SerializeValue(node, AttributeSslPort, _sslPort);
    SerializeValue(node, AttributeIsValidIp, _isValidIp);
    SerializeValue(node, AttributeIsAuthorized, _isAuthorized);
    SerializeValue(node, AttributeClientKey, _clientKey);
    SerializeValue(node, AttributeUser, _user);
    SerializeValue(node, AttributeSelectedGroup, _selectedGroup);
    SerializeValue(node, AttributeMaxNoStreamingSessions, _maxNoStreamingSessions);
    SerializeValue(node, AttributeCertificate, _certificate);
}

void Bridge::DeserializeBase(JSONNode *node) {
    Serializable::Deserialize(node);

    DeserializeValue(node, AttributeName, &_name, "");
    DeserializeValue(node, AttributeModelId, &_modelId, "");
    DeserializeValue(node, AttributeApiversion, &_apiversion, "");
    DeserializeValue(node, AttributeSwversion, &_swversion, "");
    DeserializeValue(node, AttributeId, &_id, "");
    DeserializeValue(node, AttributeIpAddress, &_ipAddress, "");
    DeserializeValue(node, AttributeSslPort, &_sslPort, "");
    DeserializeValue(node, AttributeIsValidIp, &_isValidIp, false);
    DeserializeValue(node, AttributeIsAuthorized, &_isAuthorized, false);
    DeserializeValue(node, AttributeClientKey, &_clientKey, "");
    DeserializeValue(node, AttributeUser, &_user, "");
    DeserializeValue(node, AttributeSelectedGroup, &_selectedGroup, "");
    DeserializeValue(node, AttributeMaxNoStreamingSessions, &_maxNoStreamingSessions, 0);
    DeserializeValue(node, AttributeCertificate, &_certificate, "");

    // Keep Clipv1 compatibility by assigning the app id to the same value than user id
    SetAppId(_user);
}

BridgePtr Bridge::Clone() const {
    auto bridgeCopy = std::make_shared<Bridge>(*this);
    bridgeCopy->SetGroups(clone_list(_groups));
    bridgeCopy->SetZones(clone_list(_zones));
    return bridgeCopy;
}

void Bridge::OnUserChange()
{
    // Keep Clipv1 compatibility by assigning the app id to the same value than the user id
    if (!IsSupportingClipV2())
    {
        SetAppId(GetUser());
    }
}

}  // namespace huestream
