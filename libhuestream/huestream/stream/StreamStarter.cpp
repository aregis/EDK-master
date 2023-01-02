/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <huestream/stream/StreamStarter.h>
#include <libjson/libjson.h>
#include <huestream/config/Config.h>

#include <sstream>
#include <string>
#include <memory>

namespace huestream {

#define CLIP_ERROR_TYPE_UNAUTHORIZED_USER 1
#define CLIP_ERROR_TYPE_RESOURCE_NOT_AVAILABLE 3
#define CLIP_ERROR_TYPE_PARAMETER_NOT_AVAILABLE 6
#define CLIP_ERROR_TYPE_CANT_ACTIVATE 307

#define CLIPV2_ERROR_TYPE_UNAUTHORIZED_USER 401
#define CLIPV2_ERROR_TYPE_IS_BUSY 503
#define CLIPV2_ERROR_TYPE_RESOURCE_NOT_AVAILABLE 404
#define CLIPV2_ERROR_TYPE_BODY_ERROR 400

std::string SerializeJson(const JSONNode &j) {
        return j.write();
    }

    StreamStarter::StreamStarter(BridgePtr bridge, BridgeHttpClientPtr http) : _bridge(bridge), _http(http) {}

    StreamStarter::~StreamStarter() {}

    bool StreamStarter::StartStream(ActivationOverrideLevel overrideLevel) {
        auto force = (overrideLevel != ACTIVATION_OVERRIDELEVEL_NEVER);

        if (_bridge->IsSupportingClipV2()) {
            // On clipv2 we're mostly always up to date, so check if the group is currently own by someone else and act appropriately.
            auto groupsOwnedByOtherClient = _bridge->GetGroupsOwnedByOtherClient();

            bool result = false;

            if (groupsOwnedByOtherClient->size() > 0) {
                auto g = groupsOwnedByOtherClient->at(0);

                if (force && (overrideLevel == ACTIVATION_OVERRIDELEVEL_ALWAYS || (overrideLevel == ACTIVATION_OVERRIDELEVEL_SAMEGROUP && _bridge->GetGroup()->GetId() == g->GetId()))) {
                    DeactivateGroup(g->GetId());
                }
            }
        }

        // TODO remove any direct changes to the group here, we don't need that with clipv2 because eventing will take care of it. Otherwise we should send the STREAMING_CONNECTED
        // message from here.

        auto result = Execute(true);
        if (!result && _bridge->IsBusy() && force) {
            auto groupsOwnedByOtherClient = _bridge->GetGroupsOwnedByOtherClient();
            if (overrideLevel == ACTIVATION_OVERRIDELEVEL_ALWAYS && groupsOwnedByOtherClient->size() > 0) {
                auto g = groupsOwnedByOtherClient->at(0);
                if (DeactivateGroup(g->GetId())) {
                    g->SetActive(false);
                    g->SetOwner("");
                }
            }
            Execute(false);
            result = Execute(true);
        }
        if (result) {
            _bridge->GetGroup()->SetActive(true);
            _bridge->GetGroup()->SetOwner(_bridge->GetAppId());
        } else if (_bridge->IsBusy() && _bridge->IsValidGroupSelected() && force) {
            _bridge->GetGroup()->SetActive(false);
        }
        return result;
    }

    bool StreamStarter::Start(bool force) {
        auto overrideLevel = force ? ACTIVATION_OVERRIDELEVEL_SAMEGROUP : ACTIVATION_OVERRIDELEVEL_NEVER;
        return StartStream(overrideLevel);
    }

    void StreamStarter::Stop() {
        if (Execute(false) && _bridge->IsValidGroupSelected()) {
            _bridge->GetGroup()->SetActive(false);
            _bridge->GetGroup()->SetOwner("");
        }
    }

    bool StreamStarter::DeactivateGroup(std::string groupId) {
        std::string url;
        if (_bridge->IsSupportingClipV2()) {
            url = _bridge->GetBaseUrl(true) + "/entertainment_configuration/" + groupId;
        }
        else {
            url = _bridge->GetBaseUrl() + "groups/" + groupId;
        }
        if (!Execute(url, false)) {
            return false;
        }
        auto group = _bridge->GetGroupById(groupId);
        if (group != nullptr) {
            group->SetActive(false);
            group->SetOwner("");
        }
        return true;
    }

    bool StreamStarter::Execute(bool activate) {
        auto url = _bridge->GetSelectedGroupUrl();
        return Execute(url, activate);
    }

    bool StreamStarter::Execute(std::string url, bool activate) {
        JSONNode groupNode;

                if (_bridge->IsSupportingClipV2())
                {
                        groupNode.push_back(JSONNode("action", activate ? "start" : "stop"));
                }
                else {
                        JSONNode streamNode;
                        streamNode.set_name("stream");
                        streamNode.push_back(JSONNode("active", activate));
                        groupNode.push_back(streamNode);
                }

        auto data = SerializeJson(groupNode);
        auto request = _http->ExecuteHttpRequest(_bridge, HTTP_REQUEST_PUT, url, data);


        return CheckForErrors(request);
    }

    bool StreamStarter::CheckForErrors(HttpRequestPtr req) {
        if (!req->GetSuccess()) {
            _bridge->SetIsValidIp(false);
            return false;
        }

                JSONNode node = libjson::parse(req->GetResponse());

                if (_bridge->IsSupportingClipV2())
                {
                    if (SerializerHelper::IsAttributeSet(&node, "errors"))
                    {
                        auto errorArr = node["errors"].as_array();

                        for (auto it = errorArr.begin(); it != errorArr.end(); ++it)
                        {
                            auto errorCode = it->find("http_error");

                            if (errorCode != it->end())
                            {
                                if (errorCode->as_int() == CLIPV2_ERROR_TYPE_UNAUTHORIZED_USER)
                                {
                                    _bridge->SetIsAuthorized(false);
                                }
                                else if (errorCode->as_int() == CLIPV2_ERROR_TYPE_IS_BUSY || errorCode->as_int() == CLIPV2_ERROR_TYPE_BODY_ERROR)
                                {
                                    _bridge->SetIsBusy(true);
                                }

                                return false;
                            }
                        }
                    }
                }
                else
                {
                    if (node.type() != JSON_ARRAY)
                    {
                        return false;
                    }

                    for (auto it = node.begin(); it != node.end(); ++it)
                    {
                        auto entry = it->find("error");
                        if (entry != it->end())
                        {
                            auto i = entry->find("type");
                            if (i != entry->end())
                            {
                                if (i->as_int() == CLIP_ERROR_TYPE_UNAUTHORIZED_USER)
                                {
                                    _bridge->SetIsAuthorized(false);
                                }
                                if (IsNotExistingOrInvalidGroup(static_cast<int>(i->as_int())))
                                {
                                    _bridge->DeleteGroup(_bridge->GetSelectedGroup());
                                }
                                if (i->as_int() == CLIP_ERROR_TYPE_CANT_ACTIVATE)
                                {
                                    _bridge->SetIsBusy(true);
                                }
                            }
                            return false;
                        }
                    }
                }

        _bridge->SetIsBusy(false);

        return true;
    }

    bool StreamStarter::IsNotExistingOrInvalidGroup(int errorCode) {
        return (errorCode == CLIP_ERROR_TYPE_RESOURCE_NOT_AVAILABLE ||
                errorCode == CLIP_ERROR_TYPE_PARAMETER_NOT_AVAILABLE);
    }

}  // namespace huestream
