/*******************************************************************************
Copyright (C) 2020 Signify Holding
All Rights Reserved.
********************************************************************************/

#include "huestream/connect/BridgeConfigRetriever.h"
#include <cctype>

using namespace huestream;

#define CLIPV1_ERROR_TYPE_UNAUTHORIZED_USER 1
#define CLIPV2_ERROR_TYPE_UNAUTHORIZED_USER 401

#ifdef SUPPORT_CUSTOM_MEMORY_BLOCK_DUMPING_ON_CRASH
#include <intsafe.h>
extern void SetMemoryBlockToDump(ULONG64 aBaseAddress, ULONG aSize);
#endif

BridgeConfigRetriever::BridgeConfigRetriever(const BridgeHttpClientPtr http, bool useForcedActivation):
    _http(http),
    _useForcedActivation(useForcedActivation),
    _connected(false),
    _refreshing(false),
    _busy(false),
    _sendFeedback(false),
    _bridge(nullptr),
    _eventRequestId(-1),
    _nextSendFeedbackIsScheduled(false),
    _canSendStreamingDisconnectedEvent(true),
    _sendConnectFeedback(false) {
    _resourceInfoList.push_back({"/device", [this](JSONNode& node){AddDevice(node);}});
    _resourceInfoList.push_back({"/zigbee_connectivity", [this](JSONNode& node){AddZigbeeConnectivity(node);}});
    _resourceInfoList.push_back({"/entertainment", [this](JSONNode& node) {AddEntertainment(node); } });
    _resourceInfoList.push_back({"/bridge", [this](JSONNode& node){AddBridge(node);}});
    _resourceInfoList.push_back({"/light", [this](JSONNode& node){AddLight(node);}});
    _resourceInfoList.push_back({"/zone", [this](JSONNode& node){AddZone(node);}});
    _resourceInfoList.push_back({"/entertainment_configuration", [this](JSONNode& node){AddEntertainmentConfiguration(node);}});
    //_resourceInfoList.push_back({"/grouped_light", [this](JSONNode& node){AddGroupedLight(node);}});
    _resourceInfoList.push_back({"/scene", [this](JSONNode& node){AddScene(node);}});
    _executor = support::make_unique<support::SynchronousExecutor>(support::GlobalThreadPool::get());
}

BridgeConfigRetriever::~BridgeConfigRetriever()
{
    _executor->shutdown();    

    Abort();
}

bool BridgeConfigRetriever::Execute(BridgePtr bridge, RetrieveCallbackHandler cb, FeedbackHandler fh) {
    bool busy = false;

    uint32_t sleepDuration = 0;

    while (!_busy.compare_exchange_strong(busy, true) && sleepDuration < 10000)
    {
        busy = false;
        // Sleep a bit until we're not busy anymore, and make sure we're not doing that forever.
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        sleepDuration += 5;
    }

    if (sleepDuration >= 10000) {
        cb(OPERATION_FAILED, bridge);
        return false;
    }

    return DoExecute(bridge->Clone(), cb, fh);
}

bool BridgeConfigRetriever::DoExecute(BridgePtr bridge, RetrieveCallbackHandler cb, FeedbackHandler fh)
{
    // In case we try to fetch a different bridge than the current one, just clear everything and start from scratch
    if (_bridge != nullptr && _bridge->GetId() != bridge->GetId())
    {
        OnBridgeDisconnect();
    }

    // Otherwise there's probably no need to fetch the full config again. Since we're eventing, our bridge object is suppose to be up to date,
    // unless we were unable to connect first. Also don't get full config if we are already getting it by refreshing.
    bool connected = false;
    if (!_connected.compare_exchange_strong(connected, true))
    {
        if (!_bridge->IsAuthorized() || !_bridge->IsValidIp())
        {
            if (cb != nullptr)
            {
                cb(OPERATION_FAILED, _bridge);
            }
        }
        else
        {
            if (cb != nullptr)
            {
                cb(OPERATION_SUCCESS, _bridge);
            }
        }

        _busy = false;
        return false;
    }

    _bridge = bridge;
    _cb = cb;
    _fh = fh;
    GroupPtr selectedGroup = _bridge->GetGroup();
    _previouslySelectedGroupName = selectedGroup == nullptr ? "" : selectedGroup->GetName();

    _allDeviceMap.clear();
    _allLightMap.clear();
    _allZigbeeConnetivityMap.clear();
    _allZoneMap.clear();
    _allEntertainment.clear();
    _allSceneMap.clear();
    _whitelist.clear();

    // Get white list first on ClipV1 because it's not available yet on ClipV2
    UpdateWhitelist([this]()
    {
        // Get application id because on Clipv2 the group's owner is set to it while streaming. So the bridge will need it to infer group's ownership.
        GetApplicationId([this]()
        {
            // Then get the full config
            GetFullConfig();
        });
    });

    return true;
}

void BridgeConfigRetriever::Abort()
{
    if (_eventRequestId != -1)
    {
        _http->CancelHttpRequest(_eventRequestId);
        _eventRequestId = -1;
    }
}

void BridgeConfigRetriever::OnBridgeMonitorEvent(const FeedbackMessage& message)
{
    if (message.GetId() == FeedbackMessage::ID_BRIDGE_DISCONNECTED)
    {
        OnBridgeDisconnect();
    }
    else if (message.GetId() == FeedbackMessage::ID_USERPROCEDURE_STARTED && (message.GetRequestType() == FeedbackMessage::REQUEST_TYPE_DEACTIVATE || message.GetRequestType() == FeedbackMessage::REQUEST_TYPE_ACTIVATE))
    {
      // Don't send streaming disconnect event in case a manual procedure has been started, otherwise there's a risk that a duplicate event might be sent. 
      // Also in case of an activate, when a previous session was not cleanly closed (ex crash), we don't want to throw a streaming stop event since
      // we're going to start streaming immediately. If streaming start failed for whatever reason, the streaming stop event is going to get thrown by the "Final" function of "ConnectionFlow".
      _canSendStreamingDisconnectedEvent = false;
    }
    else if (message.GetId() == FeedbackMessage::ID_USERPROCEDURE_FINISHED && (message.GetRequestType() == FeedbackMessage::REQUEST_TYPE_DEACTIVATE || message.GetRequestType() == FeedbackMessage::REQUEST_TYPE_ACTIVATE))
    {
      // Allow sending of streaming disconnect event when manual procedure is finished
      _canSendStreamingDisconnectedEvent = true;
    }
}

void BridgeConfigRetriever::RefreshBridgeConnection()
{
    bool busy = false;
    if (!_busy.compare_exchange_strong(busy, true))
    {
        // If we're already fetching the config just return.
        return;
    }

    // Refresh only if already connected
    bool connected = true;
    if (_refreshing || !_connected.compare_exchange_strong(connected, false))
    {
        return;
    }

    // Cancel our http stream request, otherwise nothing will happen
    Abort();

    _sendFeedback = false;
    _sendConnectFeedback = false;
    _refreshing = true;

    if (!DoExecute(_bridge->Clone(), nullptr, _fh))
    {
        _refreshing = false;
    }
}

void BridgeConfigRetriever::OnBridgeDisconnect()
{
    bool connected = true;
    if (!_connected.compare_exchange_strong(connected, false))
    {
        return;
    }

    // Cancel our http stream request, otherwise nothing will happen
    Abort();

    _sendFeedback = false;
    _sendConnectFeedback = true;
}

void BridgeConfigRetriever::GetFullConfig()
{
    // Clear groups and zones otherwise we might get duplicates
    _bridge->GetGroups()->clear();
    _bridge->GetZones()->clear();

    // Then get all the individual resources we need
    _resourceToGet = _resourceInfoList.begin();

    if (_resourceToGet != _resourceInfoList.end())
    {
        GetResource("");
    }
}

void BridgeConfigRetriever::OnGetResource()
{
    JSONNode n = libjson::parse(_response);

    if (!SerializerHelper::IsAttributeSet(&n, "data"))
    {
        _bridge->SetIsValidIp(false);
        Finish(OPERATION_FAILED);
        return;
    }

    auto data = n["data"].as_array();
    for (auto resourceIt = data.begin(); resourceIt != data.end(); ++resourceIt)
    {
        std::get<1>(*_resourceToGet)(*resourceIt);
    }

    std::string cursor = "";

    // Check for a cursor, which means there's more data available for that resource
    if (SerializerHelper::IsAttributeSet(&n, "cursor"))
    {
        cursor = "?cursor=";
        cursor += n["cursor"].as_string();
    }
    else
    {
        ++_resourceToGet;
    }

    if (_resourceToGet != _resourceInfoList.end())
    {
        GetResource(cursor);
    }
    else
    {
        // At the end make sure to translate bridge currently selected group from id_v1 to the new ClipV2 id.
        // Otherwise an app might start with no selected group which could break functionality. However since
        // group id_v1 is not available anymore on clipv2, compare names instead.
        if (_cb != nullptr && !_bridge->GetSelectedGroup().empty() && GetGroupById(_bridge->GetSelectedGroup()) == nullptr)
        {
            GroupPtr selectedGroup = GetGroupByName(_previouslySelectedGroupName);

            if (selectedGroup != nullptr)
            {
                _bridge->SetSelectedGroup(selectedGroup->GetId());
            }
        }

        Finish(OPERATION_SUCCESS, false);

        _cb = nullptr;
        _sendFeedback = true;

        if (_refreshing)
        {
            _fh(FeedbackMessage(FeedbackMessage::REQUEST_TYPE_INTERNAL, FeedbackMessage::ID_BRIDGE_REFRESHED, _bridge));

            // In that case we send the following update events regardless if there's been a change or not. Note that it could always be possible to compare
            // the old bridge with the new one and only send the relevant events. However since refreshing is only suppose to happen when the eventing connection
            // becomes invalid, it's not worth the effort.
            _fh(FeedbackMessage(FeedbackMessage::REQUEST_TYPE_INTERNAL, FeedbackMessage::ID_ZONELIST_UPDATED, _bridge));
            _fh(FeedbackMessage(FeedbackMessage::REQUEST_TYPE_INTERNAL, FeedbackMessage::ID_GROUPLIST_UPDATED, _bridge));
            _fh(FeedbackMessage(FeedbackMessage::REQUEST_TYPE_INTERNAL, FeedbackMessage::ID_ZONE_SCENELIST_UPDATED, _bridge));

            if (!_bridge->GetSelectedGroup().empty() && GetGroupById(_bridge->GetSelectedGroup()) == nullptr)
      {
                _bridge->SetSelectedGroup("");
        _fh(FeedbackMessage(FeedbackMessage::REQUEST_TYPE_INTERNAL, FeedbackMessage::ID_SELECT_GROUP, _bridge));
      }

            _refreshing = false;
        }

        // When everything is done, listen to various events from the server
        ListenToEvents();

        _busy = false;
    }
}

void BridgeConfigRetriever::GetResource(const std::string& cursor)
{
    std::string url = _bridge->GetBaseUrl(true, false);
    url += std::get<0>(*_resourceToGet);

    if (!cursor.empty())
    {
        url += cursor;
    }

    std::weak_ptr<BridgeConfigRetriever> lifetime = shared_from_this();

    _http->ExecuteHttpRequest(_bridge, HTTP_REQUEST_GET, url, "", [lifetime, this](const support::HttpRequestError& error, const support::IHttpResponse& response)
    {
        std::shared_ptr<BridgeConfigRetriever> ref = lifetime.lock();
        if (ref == nullptr)
        {
            _busy = false;
            _refreshing = false;
            return;
        }

        bool success = ValidateHttpRequestStatus(error, response);

        if (success)
        {
            _response = response.get_body();
            OnGetResource();
        }
        else
        {
            Finish(OPERATION_FAILED);
        }
    });
}

void BridgeConfigRetriever::UpdateWhitelist(std::function<void()> callback)
{
    std::string configUrl = _bridge->GetBaseUrl(false, false);
    configUrl += "config";

    std::weak_ptr<BridgeConfigRetriever> lifetime = shared_from_this();

    _http->ExecuteHttpRequest(_bridge, HTTP_REQUEST_GET, configUrl, "", [lifetime, this, callback](const support::HttpRequestError& error, const support::IHttpResponse& response)
    {
        std::shared_ptr<BridgeConfigRetriever> ref = lifetime.lock();
        if (ref == nullptr)
        {
            _refreshing = false;
            _busy = false;
            return;
        }

        bool hasError = false;
        bool hasUnauthorizedError = false;
        JSONNode root;

        if (error.get_code() != support::HttpRequestError::HTTP_REQUEST_ERROR_CODE_SUCCESS)
        {
            hasError = true;
        }
        else
        {
            root = libjson::parse(response.get_body());

            if (root.type() == JSON_NULL)
            {
                hasError = true;
            }
            else if (root.type() == JSON_ARRAY)
            {
                for (auto it = root.begin(); it != root.end(); ++it)
                {
                    auto entry = it->find("error");
                    if (entry != it->end())
                    {
                        hasError = true;
                        auto i = entry->find("type");
                        if (i != entry->end())
                        {
                            if (i->as_int() == CLIPV1_ERROR_TYPE_UNAUTHORIZED_USER)
                            {
                                hasUnauthorizedError = true;
                            }
                        }
                        break;
                    }
                }
            }
            else if (!SerializerHelper::IsAttributeSet(&root, "whitelist"))
            {
                // Make sure white list is there, this is happening when the user is not authorized. In that case the http call succeed and we only get a partial config without the white list.
                hasError = true;
                hasUnauthorizedError = true;
            }
        }

        if (hasError)
        {
            if (hasUnauthorizedError)
            {
                _bridge->SetIsAuthorized(false);
                _bridge->SetIsValidIp(true);
            }
            else
            {
                _bridge->SetIsValidIp(false);
            }

            Finish(OPERATION_FAILED);
            return;
        }

        _whitelist = root["whitelist"].as_node();

        // At this point we should be authorized and have a valid ip, so just make sure those flags are right.
        _bridge->SetIsAuthorized(true);
        _bridge->SetIsValidIp(true);

        callback();
    });
}

void BridgeConfigRetriever::GetApplicationId(std::function<void()> callback)
{
    std::string url = _bridge->GetAppIdUrl();

    std::weak_ptr<BridgeConfigRetriever> lifetime = shared_from_this();

    _http->ExecuteHttpRequest(_bridge, HTTP_REQUEST_GET, url, "", [lifetime, this, callback](const support::HttpRequestError& error, const support::IHttpResponse& response)
    {
        std::shared_ptr<BridgeConfigRetriever> ref = lifetime.lock();
        if (ref == nullptr)
        {
            _refreshing = false;
            _busy = false;
            return;
        }

        bool success = ValidateHttpRequestStatus(error, response);

        if (success)
        {
            const char* appId = response.get_header_field_value("hue-application-id");
            _bridge->SetAppId(appId);
            callback();
        }
        else
        {
            Finish(OPERATION_FAILED);
        }
    });
}

void BridgeConfigRetriever::ListenToEvents()
{
    std::string baseUrl = _bridge->GetBaseUrl(true, true);
    std::weak_ptr<BridgeConfigRetriever> lifetime = shared_from_this();

    // This is the eventing request which stays open and from which we receive server events from time to time. Communication is unidirectional, i.e. from server to client.
    _eventRequestId = _http->ExecuteHttpRequest(_bridge, HTTP_REQUEST_GET, baseUrl, "", [lifetime, this](const support::HttpRequestError& error, const support::IHttpResponse& response)
    {
        /*static std::chrono::steady_clock::time_point lastTime = std::chrono::steady_clock::now();
        std::chrono::steady_clock::time_point nowTime = std::chrono::steady_clock::now();
        std::chrono::nanoseconds diffTime = nowTime - lastTime;
        lastTime = nowTime;
        HUE_LOG << HUE_CORE << HUE_INFO << "TIME ELAPSED BETWEEN UPDATES: " << (diffTime.count() / 1000000000.0) << HUE_ENDL;*/

        std::shared_ptr<BridgeConfigRetriever> ref = lifetime.lock();
        if (ref == nullptr)
        {
            return;
        }

        if (error.get_code() == support::HttpRequestError::HTTP_REQUEST_ERROR_CODE_SUCCESS)
        {
            _responseEvent += response.get_body();
            ParseEventResponseAndExecuteCallback();
        }
        else if (error.get_code() != support::HttpRequestError::HTTP_REQUEST_ERROR_CODE_CANCELED)
        {
            Finish(OPERATION_FAILED);
        }
    }, true);
}

bool BridgeConfigRetriever::ValidateHttpRequestStatus(const support::HttpRequestError& error, const support::IHttpResponse& response)
{
    if (error.get_code() == support::HttpRequestError::HTTP_REQUEST_ERROR_CODE_SUCCESS)
    {
        switch (response.get_status_code())
        {
            case 200:
            {
                return true;
            }
            case 401:
            case 403:
            {
                _bridge->SetIsAuthorized(false);
                return false;
            }
            default:
            {
                _bridge->SetIsValidIp(false);
                return false;
            }
        }
    }

    return false;
}

void BridgeConfigRetriever::ParseEventResponseAndExecuteCallback() {
    do
    {
        // Look for data sequence
        size_t startPos = _responseEvent.find("data: ");

        if (startPos == std::string::npos)
        {
            return;
        }

        size_t endPos = _responseEvent.find("id: ", startPos);

        if (endPos == std::string::npos)
        {
            endPos = _responseEvent.size();
        }
        else
        {
            HUE_LOG << HUE_CORE << HUE_WARN << "ParseEventResponseAndExecuteCallback: more than one event received" << HUE_ENDL;
        }

        std::string jsonstr = _responseEvent.substr(startPos + 6, endPos - (startPos + 6));

        if (!libjson::is_valid(jsonstr)) {
            // Incomplete response so wait for the remaining
            return;
        }

        JSONNode n = libjson::parse(jsonstr);

#ifdef SUPPORT_CUSTOM_MEMORY_BLOCK_DUMPING_ON_CRASH
        SetMemoryBlockToDump(reinterpret_cast<ULONG64>(jsonstr.c_str()), static_cast<ULONG>(jsonstr.size() * sizeof(char)));
#endif

        // There might be more than 1 event available
        _responseEvent = _responseEvent.substr(endPos);

        if (!ParseJsonEvent(n))
        {
            Finish(OPERATION_FAILED);
        }
        else
        {
            Finish(OPERATION_SUCCESS);
        }
    } while (true);
}

bool BridgeConfigRetriever::ParseJsonEvent(JSONNode& root)
{
    if (root.type() != JSON_ARRAY)
    {
        // Something went wrong
        return false;
    }

    std::vector<std::tuple<std::string, JSONNode>> ecList;
    std::vector<std::tuple<std::string, JSONNode>> lightList;
    std::vector<std::tuple<std::string, JSONNode>> deviceList;
    std::vector<std::tuple<std::string, JSONNode>> zcList;
    std::vector<std::tuple<std::string, JSONNode>> zoneList;
    std::vector<std::tuple<std::string, JSONNode>> entertainmentList;
    std::vector<std::tuple<std::string, JSONNode>> groupedLightList;
    std::vector<std::tuple<std::string, JSONNode>> sceneList;
    JSONNode bridge;
    std::string bridgeEventType;

    // Create data maps first, because we need to process stuff in order and it will also be easier to cross reference various json nodes later on.
    uint32_t count = 0;
    for (auto it = root.begin(); it != root.end(); ++it)
    {
        count++;
        auto typeNode = it->find("type");
        int32_t nodeType = -1;

        if (typeNode == root.end() || typeNode->type() != JSON_STRING)
        {
            if (typeNode != root.end())
            {
                nodeType = typeNode->type();
                HUE_LOG << HUE_CORE << HUE_WARN << "ParseJsonEvent: node type is not string but " << nodeType << " iterator " << count << HUE_ENDL;
            }

            continue;
        }

        auto type = typeNode->as_string();
        auto data = it->find("data");

        if (data->type() == JSON_NULL || data == root.end())
        {
            continue;
        }

        std::string typeId = "";
        JSONNode dataArray = data->as_array();
        for (auto dataIt = dataArray.begin(); dataIt != dataArray.end(); ++dataIt)
        {
            JSONNode dataNode = dataIt->as_node();
            Serializable::DeserializeValue(&dataNode, "type", &typeId, "");

            if (typeId.empty())
            {
                continue;
            }

            std::string dataId = "";
            Serializable::DeserializeValue(&dataNode, "id", &dataId, "");

            if (dataId.empty())
            {
                continue;
            }

            if (typeId == "entertainment_configuration")
            {
               ecList.push_back({ type, dataNode });
            }
            else if (typeId == "light")
            {
                lightList.push_back({ type, dataNode });
            }
            else if (typeId == "device")
            {
               deviceList.push_back({ type, dataNode });
            }
            else if (typeId == "zigbee_connectivity")
            {
                zcList.push_back({ type, dataNode });
            }
            else if (typeId == "bridge")
            {
                bridge = dataNode;
                bridgeEventType = type;
            }
            else if (typeId == "zone")
            {
               zoneList.push_back({ type, dataNode });
            }
            else if (typeId == "entertainment")
            {
                entertainmentList.push_back({ type, dataNode });
            }
            else if (typeId == "grouped_light")
            {
                groupedLightList.push_back({ type, dataNode });
            }
            else if (typeId == "scene")
            {
                sceneList.push_back({ type, dataNode });
            }
        }
    }

    // Parse all devices by event category
    for (auto it = deviceList.begin(); it != deviceList.end(); ++it)
    {
        const std::string& eventType = std::get<0>(*it);

        if (eventType == "add")
        {
            AddDevice(std::get<1>(*it));
        }
        else if (eventType == "update")
        {
            UpdateDevice(std::get<1>(*it));
        }
        else if (eventType == "delete")
        {
            DeleteDevice(std::get<1>(*it));
        }
    }

    // Parse all Zigbee connectivity by event category
    for (auto it = zcList.begin(); it != zcList.end(); ++it)
    {
        const std::string& eventType = std::get<0>(*it);

        if (eventType == "add")
        {
            AddZigbeeConnectivity(std::get<1>(*it));
        }
        else if (eventType == "update")
        {
            UpdateZigbeeConnectivity(std::get<1>(*it));
        }
        else if (eventType == "delete")
        {
            DeleteZigbeeConnectivity(std::get<1>(*it));
        }
    }

    // Parse bridge by event category
    if (!bridge.empty())
    {
        if (bridgeEventType == "add")
        {
            AddBridge(bridge);
        }
        else if (bridgeEventType == "update")
        {
            UpdateBridge(bridge);
        }
        else if (bridgeEventType == "delete")
        {
            DeleteBridge(bridge);
        }
    }

    // KEEP ALL LIGHTS BECAUSE IF WE GET A NEW GROUP WITH A NEW LIGHT, WE"LL PROBALY GET A NEW LIGHTS EVENT FIRST AND LATER ON A NEW GROUP. BECAUSE
    // YOU PHYSICALLY NEED TO ADD A NEW LIGHT BEFORE PUTTING IT IN A NEW GROUP OR AN EXISTING GROUP

    // Parse all entertainment by event category
    for (auto it = entertainmentList.begin(); it != entertainmentList.end(); ++it)
    {
        const std::string& eventType = std::get<0>(*it);

        if (eventType == "add")
        {
            AddEntertainment(std::get<1>(*it));
        }
        else if (eventType == "update")
        {
            UpdateEntertainment(std::get<1>(*it));
        }
        else if (eventType == "delete")
        {
            DeleteEntertainment(std::get<1>(*it));
        }
    }

    // Parse all lights by event category
    for (auto it = lightList.begin(); it != lightList.end(); ++it)
    {
        const std::string& eventType = std::get<0>(*it);

        if (eventType == "add")
        {
            AddLight(std::get<1>(*it));
        }
        else if (eventType == "update")
        {
            UpdateLight(std::get<1>(*it));
        }
        else if (eventType == "delete")
        {
            DeleteLight(std::get<1>(*it));
        }
    }

    // Parse all zones by event category
    for (auto it = zoneList.begin(); it != zoneList.end(); ++it)
    {
        const std::string& eventType = std::get<0>(*it);

        if (eventType == "add")
        {
            AddZone(std::get<1>(*it));
        }
        else if (eventType == "update")
        {
            UpdateZone(std::get<1>(*it));
        }
        else if (eventType == "delete")
        {
            DeleteZone(std::get<1>(*it));
        }
    }

    // Check bridge disconnection here because there could more than one update for the same ec but with different 
    // status (inactive first and active second). If we were to check the streaming status in the update method we could end
    // up firing a wrong ID_STREAMING_DISCONNECTED.
    bool bridgeWasStreaming = _bridge->IsStreaming();

    // Parse all entertainment configuration by event category    
    for (auto it = ecList.begin(); it != ecList.end(); ++it)
    {
        const std::string& eventType = std::get<0>(*it);

        if (eventType == "add")
        {
            AddEntertainmentConfiguration(std::get<1>(*it));
        }
        else if (eventType == "update")
        {
            UpdateEntertainmentConfiguration(std::get<1>(*it));
        }
        else if (eventType == "delete")
        {
            DeleteEntertainmentConfiguration(std::get<1>(*it));
        }
    }

    // Streaming disconnected can happen here when there's been an override by another client.
    // We don't check for streaming connected event here because the event is always the result of an action from the user and the event is always fired elsewhere.
    if (_canSendStreamingDisconnectedEvent && bridgeWasStreaming && !_bridge->IsStreaming())
    {
      _fh(FeedbackMessage(FeedbackMessage::REQUEST_TYPE_INTERNAL, FeedbackMessage::ID_STREAMING_DISCONNECTED, _bridge));
    }

    // Parse all grouped light by event category
    /*for (auto it = groupedLightList.begin(); it != groupedLightList.end(); ++it)
    {
        const std::string& eventType = std::get<0>(*it);

        if (eventType == "add")
        {
            AddGroupedLight(std::get<1>(*it));
        }
        else if (eventType == "update")
        {
            UpdateGroupedLight(std::get<1>(*it));
        }
    }*/

    // Parse all scenes by event category
    for (auto it = sceneList.begin(); it != sceneList.end(); ++it)
    {
        const std::string& eventType = std::get<0>(*it);

        if (eventType == "add")
        {
            AddScene(std::get<1>(*it));
        }
        else if (eventType == "update")
        {
            UpdateScene(std::get<1>(*it));
        }
        else if (eventType == "delete")
        {
            DeleteScene(std::get<1>(*it));
        }
    }

    return true;
}

GroupPtr BridgeConfigRetriever::ParseEntertainmentConfig(const JSONNode &node)
{
    auto group = std::make_shared<Group>();
    LightListPtr physicalLightList = std::make_shared<LightList>();
    LightListPtr channelLightList = std::make_shared<LightList>();
    GroupChannelToPhysicalLightMapPtr channelToPhysicalLightsMap = std::make_shared<GroupChannelToPhysicalLightMap>();

    // TODO Look for the zone associated with this entertainment config, then look for the corresponding grouped_light and use its on and brightness state.
    //group->SetOnState
    //group->SetBrightness

    std::string id;
    Serializable::DeserializeValue(&node, "id", &id, "");
    group->SetId(id);

    ParseName(node, group);
    ParseClass(node, group);
    ParseStreamActive(node, group);
    ParseStreamProxy(node, group);
    ParseLights(node, group, physicalLightList);
    ParseChannels(node, channelLightList, channelToPhysicalLightsMap);

    group->SetPhysicalLights(physicalLightList);
    group->SetLights(channelLightList);
    group->SetChannelToPhysicalLightsMap(channelToPhysicalLightsMap);

    return group;
}

bool BridgeConfigRetriever::ParseName(const JSONNode& node, GroupPtr group)
{
    std::string name;

    if (SerializerHelper::IsAttributeSet(&node, "metadata"))
    {
        const JSONNode& metadata = node["metadata"];

        Serializable::DeserializeValue(&metadata, "name", &name, "");
    }
    else if (SerializerHelper::IsAttributeSet(&node, "name"))
    {
        Serializable::DeserializeValue(&node, "name", &name, "");
    }
    else
    {
        return false;
    }

    group->SetName(name);

    return true;
}

bool BridgeConfigRetriever::ParseClass(const JSONNode& node, GroupPtr group)
{
    if (!SerializerHelper::IsAttributeSet(&node, "configuration_type"))
    {
        return false;
    }

    std::string classType = "";
    Serializable::DeserializeValue(&node, "configuration_type", &classType, "");

    if (classType == "screen")
    {
        group->SetClassType(GROUPCLASS_SCREEN);
    }
    else if (classType == "music")
    {
        group->SetClassType(GROUPCLASS_MUSIC);
    }
    else if (classType == "3dspace")
    {
        group->SetClassType(GROUPCLASS_3DSPACE);
    }
    else
    {
        group->SetClassType(GROUPCLASS_OTHER);
    }

    return true;
}

bool BridgeConfigRetriever::ParseStreamActive(const JSONNode &node, GroupPtr group)
{
    bool parsed = false;

    if (SerializerHelper::IsAttributeSet(&node, "status"))
    {
        std::string active = node["status"].as_string();
        group->SetActive(active == "active");

        if (!group->Active())
        {
            group->SetOwner("");
            group->SetOwnerName("");
        }

        parsed = true;
    }

    if (SerializerHelper::IsAttributeSet(&node, "active_streamer"))
    {
        std::string owner = node["active_streamer"]["rid"].as_string();
        group->SetOwner(owner);
        group->SetOwnerName(GetOwnerName(owner, group->GetId()));
        parsed = true;
    }

    // Update bridge busy status
    if (!_bridge->IsStreaming())
    {
        int32_t availableSessionCount = _bridge->GetMaxNoStreamingSessions() - _bridge->GetCurrentNoStreamingSessions();
        bool selectedGroupCanBeOvertaken = _bridge->IsValidGroupSelected() && _bridge->GetGroup()->Active() && _useForcedActivation;

        bool busy = (availableSessionCount <= 0) && !selectedGroupCanBeOvertaken;
        _bridge->SetIsBusy(busy);
    }

    return parsed;
}

bool BridgeConfigRetriever::ParseStreamProxy(const JSONNode &node, GroupPtr group)
{
    if (!SerializerHelper::IsAttributeSet(&node, "stream_proxy"))
    {
        return false;
    }

    GroupProxyNode proxyNode;
    proxyNode.name = _bridge->GetName();
    proxyNode.model = _bridge->GetModelId();
    proxyNode.isReachable = true;

    auto streamProxy = node["stream_proxy"].as_node();

    if (SerializerHelper::IsAttributeSet(&streamProxy, "mode"))
    {
        proxyNode.mode = streamProxy["mode"].as_string();
    }

    if (!SerializerHelper::IsAttributeSet(&streamProxy, "node"))
    {
        return false;
    }

    auto streamProxyNode = streamProxy["node"].as_node();

    if (SerializerHelper::IsAttributeSet(&streamProxyNode, "rid"))
    {
        proxyNode.uri = streamProxyNode["rid"].as_string();
    }

    // Look for corresponding device
    std::string deviceId = GetDeviceIdFromServiceReferenceId(proxyNode.uri);

    if (deviceId.empty())
    {
        return false;
    }

    JSONNode device = _allDeviceMap[deviceId];

    std::string name;
    std::string modelId;
    bool reachable = true;
    if (SerializerHelper::IsAttributeSet(&device, "metadata"))
    {
        JSONNode metadata = device["metadata"];
        Serializable::DeserializeValue(&metadata, "name", &name, "");

        if (SerializerHelper::IsAttributeSet(&device, "product_data"))
        {
            JSONNode productData = device["product_data"];
            Serializable::DeserializeValue(&productData, "model_id", &modelId, "");

            std::string zigbeeId = GetDeviceReferenceIdFromReferenceType(deviceId, "zigbee_connectivity");
            JSONNode zigbee = _allZigbeeConnetivityMap[zigbeeId];

            if (SerializerHelper::IsAttributeSet(&zigbee, "status"))
            {
                reachable = zigbee["status"].as_string() == "connected";

                proxyNode.name = name;
                proxyNode.model = modelId;
                proxyNode.isReachable = reachable;
            }
        }
    }

    group->SetProxyNode(proxyNode);

    return true;
}

bool BridgeConfigRetriever::ParseLights(const JSONNode &node, GroupPtr group, LightListPtr lightList)
{
    if (!SerializerHelper::IsAttributeSet(&node, "locations"))
    {
        return false;
    }

    if (lightList == nullptr)
    {
        return false;
    }

    auto location = node["locations"];

    if (!SerializerHelper::IsAttributeSet(&location, "service_locations"))
    {
        return false;
    }

    double avgBri = 0.0;
    uint32_t numReachableLights = 0;
    bool on = false;

    // Parse location first, then check for associated light
    auto locationArr = location["service_locations"].as_array();

    for (json_index_t i = 0; i < locationArr.size(); i++)
    {
        auto location = locationArr[i].as_node();
        auto service = location["service"].as_node();
        std::string refId = service["rid"].as_string();

        std::string lightId = GetLightIdFromEntertainmentId(refId);

        JSONNode lightNode;
        if (!GetLightNodeById(lightId, lightNode))
        {
            continue;
        }

        LightPtr light = ParseLightInfo(lightNode);

        // Also set the light position, note that this is not necessarily the same than the channel position.
        JSONNode position(JSON_NULL);
        if (SerializerHelper::IsAttributeSet(&location, "positions"))
        {
                position = location["positions"].as_array()[0];
        }
        else if (SerializerHelper::IsAttributeSet(&location, "position"))
        {
                position = location["position"].as_node();
        }

        if (position.type() != JSON_NULL)
        {
            light->SetPosition(Location(position["x"].as_float(), position["y"].as_float(), position["z"].as_float()));
        }

        lightList->push_back(light);

        if (light->Reachable())
        {
            avgBri += light->GetBrightness();
            numReachableLights++;
            on = on || light->On();
        }
    }

    if (numReachableLights > 0)
    {
        avgBri /= numReachableLights;
    }
    group->SetBrightnessState(avgBri / 100.0);  // TODO remove when zones become available

    group->SetOnState(on); // TODO remove when zones become available

    return true;
}

LightPtr BridgeConfigRetriever::ParseLightInfo(const JSONNode& node)
{
    std::string name;
    std::string archetype;

    if (SerializerHelper::IsAttributeSet(&node, "metadata"))
    {
        JSONNode metadata = node["metadata"].as_node();
        Serializable::DeserializeValue(&metadata, "name", &name, "");
        Serializable::DeserializeValue(&metadata, "archetype", &archetype, "");
    }

    double brightness = 0.0;
    if (SerializerHelper::IsAttributeSet(&node, "dimming"))
    {
        brightness = node["dimming"].as_node()["brightness"].as_float();
    }

    auto on = node.at("on").as_node()["on"].as_bool(); // TODO remove when zones become available

    // Retrieve xy color
    double xy[2] = { 0.0, 0.0 };
    if (SerializerHelper::IsAttributeSet(&node, "color"))
    {
        auto color = node["color"];

        if (SerializerHelper::IsAttributeSet(&color, "xy"))
        {
            auto xyData = color["xy"].as_node();
            xy[0] = xyData["x"].as_float();
            xy[1] = xyData["y"].as_float();
        }
    }

    // Get the model id from the associated device
    std::string id = node["id"].as_string();
    std::string modelId;

    // Check if the light support dynamic features.
    bool dynamic = false;
    bool dynamicEnabled = false;
    if (SerializerHelper::IsAttributeSet(&node, "dynamics"))
    {
        auto dynamicNode = node["dynamics"];

        if (SerializerHelper::IsAttributeSet(&dynamicNode, "status_values"))
        {
            auto dynamicStatusValueArr = dynamicNode["status_values"].as_array();
            for (json_index_t i = 0; i < dynamicStatusValueArr.size(); ++i)
            {
                if (dynamicStatusValueArr[i].as_string() == "dynamic_palette")
                {
                    dynamic = true;
                    break;
                }
            }
        }
        if (SerializerHelper::IsAttributeSet(&dynamicNode, "status"))
        {
            auto dynamicStatus = dynamicNode["status"].as_string();

            dynamicEnabled = dynamicStatus != "none";
        }
    }

    // Look for device with associated light service to get the model id
    for (auto it = _allDeviceMap.begin(); it != _allDeviceMap.end(); it++)
    {
        auto deviceData = it->second;

        auto serviceArr = deviceData["services"].as_array();

        bool found = false;
        for (json_index_t i = 0; i < serviceArr.size(); i++)
        {
            if (serviceArr[i].as_node()["rid"] == id)
            {
                found = true;
                break;
            }
        }

        if (found)
        {
            modelId = deviceData["product_data"].as_node()["model_id"].as_string();
            break;
        }
    }

    LightPtr lightInfo = std::make_shared<Light>();
    lightInfo->SetId(id);
    lightInfo->SetIdV1(GetIdV1(node));
    lightInfo->SetName(name);
    lightInfo->SetModel(modelId);
    lightInfo->SetArchetype(archetype);
    lightInfo->SetBrightness(brightness);
    lightInfo->SetColor({xy, brightness, 100.0});
    lightInfo->SetOn(on);
    lightInfo->SetDynamic(dynamic);
    lightInfo->SetDynamicEnabled(dynamicEnabled);

    std::string zcId = GetZigbeeConnectivityIdFromLightId(id);
    JSONNode zc = _allZigbeeConnetivityMap[zcId];
    lightInfo->SetReachable(zc["status"].as_string() == "connected");

    return lightInfo;
}

bool BridgeConfigRetriever::ParseChannels(const JSONNode &node, LightListPtr lightList, GroupChannelToPhysicalLightMapPtr channelToPhysicalLightsMap)
{
    if (!SerializerHelper::IsAttributeSet(&node, "channels"))
    {
        return false;
    }

    if (lightList == nullptr)
    {
        return false;
    }

    auto channelArr = node["channels"].as_array();

    for (json_index_t i = 0; i < channelArr.size(); i++)
    {
        auto channel = channelArr[i].as_node();
        auto channelId = static_cast<int32_t>(channel["channel_id"].as_int());
        auto position = channel["position"].as_node();

        // Fetch the lights associated with this channel
        if (!SerializerHelper::IsAttributeSet(&channel, "members"))
        {
            continue;
        }

        LightPtr lightChannel = std::make_shared<Light>();
        lightChannel->SetId(std::to_string(channelId));
        lightChannel->SetPosition(Location(position["x"].as_float(), position["y"].as_float(), position["z"].as_float()));

        lightList->push_back(lightChannel);

        // Generate channel - physical lights map
        auto membersArr = channel["members"].as_array();

        std::vector<std::string> physicalLightIdList;

        for (json_index_t j = 0; j < membersArr.size(); ++j)
        {
            auto member = membersArr[j].as_node();
            auto service = SerializerHelper::GetAttributeValue(&member, "service");
            auto rid = SerializerHelper::GetAttributeValue(&service, "rid");

            if (rid.type() == JSON_STRING)
            {
                std::string lightId = GetLightIdFromEntertainmentId(rid.as_string());

                if (!lightId.empty())
                {
                    physicalLightIdList.push_back(lightId);
                }
            }
        }

        channelToPhysicalLightsMap->insert({ std::to_string(channelId), physicalLightIdList });
    }

    return true;
}

bool BridgeConfigRetriever::ParseGroupedLight(const JSONNode& node)
{
    // TODO The grouped light id should be set when we parse the entertainment configuration, but
    // until a zone is linked to it on the bridge there's nothing we can do. Also zone on/off state
    // won't be parse here, but it should when entertainment_configuration are linked to a zone.

    // We're only interested in the "on" property here
    if (!SerializerHelper::IsAttributeSet(&node, "id_v1") || !SerializerHelper::IsAttributeSet(&node, "on"))
    {
        return false;
    }

    std::string idv1 = node["id_v1"].as_string();

    GroupPtr group = GetGroupByIdV1(idv1);

    if (group == nullptr)
    {
        return false;
    }

    group->SetOnState(node["on"]["on"].as_bool());

    return true;
}

bool BridgeConfigRetriever::ParseScene(const JSONNode& node, ScenePtr& scene)
{
    std::string id;
    Serializable::DeserializeValue(&node, "id", &id, "");
    std::string idv1 = GetIdV1(node);

    if (id.empty() || idv1.empty())
    {
        return false;
    }

    JSONNode metadata = SerializerHelper::GetAttributeValue(&node, "metadata");
    JSONNode name = SerializerHelper::GetAttributeValue(&metadata, "name");
    JSONNode image = SerializerHelper::GetAttributeValue(&metadata, "image");

    if (scene == nullptr && (name.type() != JSON_STRING || image.type() != JSON_NODE))
    {
        // Cannot create a scene without those fields
        return false;
    }

    std::string imageId;

    JSONNode imageRefId = SerializerHelper::GetAttributeValue(&image, "rid");

    if (imageRefId.type() == JSON_STRING)
    {
        imageId = imageRefId.as_string();
    }

    // Parse actions
    JSONNode actions = SerializerHelper::GetAttributeValue(&node, "actions");
    LightListPtr lightList = std::make_shared<LightList>();

    if (actions.type() == JSON_ARRAY)
    {
        auto actionArr = actions.as_array();

        for (json_index_t i = 0; i < actionArr.size(); i++)
        {
            JSONNode item = actionArr[i].as_node();
            JSONNode action = item["action"];
            JSONNode target = item["target"];

            // Get the associated light
            JSONNode targetType = SerializerHelper::GetAttributeValue(&target, "rtype");

            if (targetType.type() != JSON_STRING || targetType.as_string() != "light")
            {
                continue;
            }

            JSONNode rid = SerializerHelper::GetAttributeValue(&target, "rid");

            if (rid.type() != JSON_STRING)
            {
                continue;
            }

            JSONNode lightNode;
            if (!GetLightNodeById(rid.as_string(), lightNode))
            {
                continue;
            }

            LightPtr light = ParseLightInfo(lightNode);

            double bri = 0.0;
            bool on = false;

            if (SerializerHelper::IsAttributeSet(&action, "dimming"))
            {
                bri = action["dimming"]["brightness"].as_float();
            }

            if (SerializerHelper::IsAttributeSet(&action, "on"))
            {
                on = action["on"]["on"].as_bool();
            }

            if (SerializerHelper::IsAttributeSet(&action, "color"))
            {
                auto color = action["color"];

                if (SerializerHelper::IsAttributeSet(&color, "xy"))
                {
                    JSONNode xyNode = color["xy"];

                    double xy[2];
                    xy[0] = xyNode["x"].as_float();
                    xy[1] = xyNode["y"].as_float();

                    light->SetColor(Color(xy, bri, 100));
                }
            }
            else if (SerializerHelper::IsAttributeSet(&action, "color_temperature"))
            {
                int ct = static_cast<int32_t>(action["color_temperature"]["mirek"].as_int());

                light->SetColor(Color(ct, bri, 100));
            }
            else
            {
                light->SetColor(Color(0.0, 0.0, 0.0));
            }

            light->SetBrightness(bri);
            light->SetOn(on);

            lightList->push_back(light);
        }
    }

    // Check if scene is dynamic by parsing the palette if available
    bool dynamic = false;
    bool dynamicUpdated = false;
    if (SerializerHelper::IsAttributeSet(&node, "palette"))
    {
        JSONNode palette = SerializerHelper::GetAttributeValue(&node, "palette");

        if (SerializerHelper::IsAttributeSet(&palette, "color"))
        {
            dynamic = SerializerHelper::GetAttributeValue(&palette, "color").as_array().size() != 0;
            dynamicUpdated = true;
        }
        if (!dynamic && SerializerHelper::IsAttributeSet(&palette, "color_temperature"))
        {
            dynamic = SerializerHelper::GetAttributeValue(&palette, "color_temperature").as_array().size() != 0;
            dynamicUpdated = true;
        }
        if (!dynamic && SerializerHelper::IsAttributeSet(&palette, "brightness"))
        {
            dynamic = SerializerHelper::GetAttributeValue(&palette, "brightness").as_array().size() != 0;
            dynamicUpdated = true;
        }
    }

    bool updated = false;

    if (scene == nullptr)
    {
        std::string nameStr = name.as_string();
        scene = std::make_shared<Scene>(id, idv1, nameStr, "", imageId);
        scene->SetLights(lightList);
        scene->SetDynamic(dynamic);
        updated = true;
    }
    else
    {
        if (name.type() == JSON_STRING)
        {
            updated = true;
            scene->SetName(name.as_string());
        }

        if (!imageId.empty())
        {
            scene->SetImage(imageId);
            updated = true;
        }

        if (!lightList->empty())
        {
            scene->SetLights(lightList);
            updated = true;
        }

        if (dynamicUpdated)
        {
            scene->SetDynamic(dynamic);
            updated = true;
        }
    }

    return updated;
}

bool BridgeConfigRetriever::ParseZoneLights(const JSONNode &node, LightListPtr lightList)
{
    JSONNode children;

    if (SerializerHelper::IsAttributeSet(&node, "children"))
    {
        children = SerializerHelper::GetAttributeValue(&node, "children");
    }
    else
    {
        children = SerializerHelper::GetAttributeValue(&node, "services");
    }

    if (children.type() != JSON_ARRAY)
    {
        return false;
    }

    auto lightArr = children.as_array();

    for (json_index_t i = 0; i < lightArr.size(); i++)
    {
        if (lightArr[i].as_node()["rtype"] == "light")
        {
            // Parse full light info
            std::string lightId = lightArr[i].as_node()["rid"].as_string();
            JSONNode lightNode;

            if (!GetLightNodeById(lightId, lightNode))
            {
                continue;
            }

            LightPtr light = ParseLightInfo(lightNode);
            lightList->push_back(light);
        }
    }

    return true;
}

std::string BridgeConfigRetriever::GetOwnerName(const std::string& userName, const std::string& ecId) {
    std::string name;
    static bool updatingWhiteList = false;

    if (SerializerHelper::IsAttributeSet(&_whitelist, userName))
    {
        JSONNode user = (_whitelist)[userName];
        Serializable::DeserializeValue(&user, "name", &name, "");
    }
    else if (!updatingWhiteList)
    {
        // Refresh the white list since it's a new name. TODO: remove when ClipV2 white list is available...
        updatingWhiteList = true;
        std::weak_ptr<BridgeConfigRetriever> lifetime = shared_from_this();

        UpdateWhitelist([this, lifetime, ecId, userName]()
        {
            std::shared_ptr<BridgeConfigRetriever> ref = lifetime.lock();
            if (ref == nullptr)
            {
                return;
            }

            GroupPtr group = GetGroupById(ecId);

            if (group == nullptr)
            {
                // Should not happen
                updatingWhiteList = false;
                return;
            }

            std::string ownerName = GetOwnerName(userName, ecId);
            group->SetOwnerName(ownerName);

            if (_sendFeedback && !ownerName.empty())
            {
                _fh(FeedbackMessage(FeedbackMessage::REQUEST_TYPE_INTERNAL, FeedbackMessage::ID_GROUPLIST_UPDATED, _bridge));
            }

            updatingWhiteList = false;
        });
    }

    return name;
}

std::string BridgeConfigRetriever::GetIdV1(const JSONNode& node) const
{
    std::string idv1;
    Serializable::DeserializeValue(&node, "id_v1", &idv1, "");
    size_t pos = idv1.find_last_of('/');
    if (pos != std::string::npos)
    {
        idv1 = idv1.substr(pos + 1);
        return idv1;
    }

    return "";
}

void BridgeConfigRetriever::Finish(OperationResult result, bool unBusy) {
    if (result == OPERATION_FAILED && _cb != nullptr) {
        _connected = false;
    }

    if (_sendConnectFeedback && _connected)
    {
        _fh(FeedbackMessage(FeedbackMessage::REQUEST_TYPE_INTERNAL, FeedbackMessage::ID_BRIDGE_CONNECTED, _bridge));
        _sendConnectFeedback = false;
    }

    if (_cb != nullptr)
    {
        _cb(result, _bridge);
    }

    if (unBusy)
    {
        _busy = false;
        _refreshing = false;
    }
}

void BridgeConfigRetriever::ScheduleFeedback(FeedbackMessage::Id msg)
{
    std::lock_guard<std::mutex> lock(_feedbackLock);

    _feedbackMessageMap[msg] = true;

    _lastFeedbackTime = std::chrono::steady_clock::now();

    if (_nextSendFeedbackIsScheduled)
    {
        return;
    }

    DispatchFeedback();
}

void BridgeConfigRetriever::DispatchFeedback()
{
    _nextSendFeedbackIsScheduled = true;

    _executor->schedule(std::chrono::steady_clock::now() + std::chrono::milliseconds(250), [this]()
    {
        std::lock_guard<std::mutex> lock(_feedbackLock);

        if ((std::chrono::steady_clock::now() - _lastFeedbackTime) > std::chrono::milliseconds(250))
        {
            for (auto msgIt = _feedbackMessageMap.begin(); msgIt != _feedbackMessageMap.end(); ++msgIt)
            {
                _fh(FeedbackMessage(FeedbackMessage::REQUEST_TYPE_INTERNAL, static_cast<FeedbackMessage::Id>(msgIt->first), _bridge));
            }

            _feedbackMessageMap.clear();
            _nextSendFeedbackIsScheduled = false;
        }
        else
        {
            DispatchFeedback();
        }
    });
}

void BridgeConfigRetriever::AddDevice(JSONNode& device)
{
    std::string id = device["id"].as_string();

    // Only keep light devices and bridge
    auto serviceArr = device["services"].as_array();

    bool found = false;
    for (json_index_t i = 0; i < serviceArr.size(); i++)
    {
        std::string referenceType = serviceArr[i]["rtype"].as_string();
        if (referenceType == "light" || referenceType == "bridge")
        {
            found = true;
            break;
        }
    }

    if (found)
    {
        _allDeviceMap[id] = device;
    }
}

void BridgeConfigRetriever::UpdateDevice(JSONNode& device)
{
    std::string id = device["id"].as_string();
    std::unordered_map<std::string, JSONNode>::iterator it = _allDeviceMap.find(id);

    if (it == _allDeviceMap.end())
    {
        HUE_LOG << HUE_CORE << HUE_WARN << "UpdateDevice: device not found: " << id << HUE_ENDL;
        return;
    }

    JSONNode& curDevice = it->second;

    // The only thing we need from a device is the model_id and it won't change so no need to update anything.

    // If device is a bridge then, check if the name attribute has change.
    // Otherwise the only other attribute we need from a device is the model_id and it won't change so no need to update anything.
    bool isABridge = false;
    auto serviceArr = curDevice["services"].as_array();
    for (json_index_t i = 0; i < serviceArr.size(); i++)
    {
        std::string referenceType = serviceArr[i]["rtype"].as_string();
        if (referenceType == "bridge")
        {
            isABridge = true;
            break;
        }
    }

    if (isABridge && SerializerHelper::IsAttributeSet(&device, "metadata"))
    {
        JSONNode metadata = device["metadata"];
        std::string name;

        if (metadata != JSON_NULL)
        {
            Serializable::DeserializeValue(&metadata, "name", &name, "");
        }

        if (_bridge->GetName() != name)
        {
            _bridge->SetName(name);
            curDevice["metadata"]["name"] = name;

            _fh(FeedbackMessage(FeedbackMessage::REQUEST_TYPE_INTERNAL, FeedbackMessage::ID_BRIDGE_CHANGED, _bridge));
        }
    }
}

void BridgeConfigRetriever::DeleteDevice(JSONNode& device)
{
    std::string id = device["id"].as_string();

    _allDeviceMap.erase(id);
}

void BridgeConfigRetriever::AddZigbeeConnectivity(const JSONNode& zc)
{
    std::string id = zc["id"].as_string();

    // Only keep light and bridge connectivity
    bool found = false;
    for (auto deviceIt = _allDeviceMap.begin(); deviceIt != _allDeviceMap.end(); deviceIt++)
    {
        auto serviceArr = deviceIt->second["services"].as_array();

        for (json_index_t i = 0; i < serviceArr.size(); i++)
        {
            if (serviceArr[i].as_node()["rid"] == id)
            {
                found = true;
                break;
            }
        }

        if (found)
        {
            break;
        }
    }

    if (found)
    {
        _allZigbeeConnetivityMap[id] = zc;
    }
}

void BridgeConfigRetriever::UpdateZigbeeConnectivity(const JSONNode& zc)
{
    // The only thing we need from the Zigbee connectivity is the status, so make sure to update the light if there's a change
    std::string id = zc["id"].as_string();

    std::unordered_map<std::string, JSONNode>::iterator it = _allZigbeeConnetivityMap.find(id);

    if (it == _allZigbeeConnetivityMap.end() || !SerializerHelper::IsAttributeSet(&zc, "status"))
    {
        HUE_LOG << HUE_CORE << HUE_WARN << "UpdateZigbeeConnectivity: zigbee connectivity not found: " << id << HUE_ENDL;
        return;
    }

    JSONNode& curZC = it->second;
    curZC["status"] = zc["status"].as_string();

    bool lightIsReachable = curZC["status"].as_string() == "connected";

    std::string lightId = GetLightIdFromZigbeeConnectivityId(id);

    if (lightId.empty())
    {
        return;
    }

    // Now update the light if present for each group
    GroupListPtr groupList = _bridge->GetGroups();

    bool lightIsPresentInCurrentGroup = false;

    if (groupList == nullptr)
    {
        // Should not happen
        return;
    }

    for (auto groupIt = groupList->begin(); groupIt != groupList->end(); groupIt++)
    {
        LightListPtr lightList = (*groupIt)->GetPhysicalLights();

        for (auto lightIt = lightList->begin(); lightIt != lightList->end(); lightIt++)
        {
            if ((*lightIt)->GetId() == lightId)
            {
                (*lightIt)->SetReachable(lightIsReachable);

                if ((*groupIt) == _bridge->GetGroup())
                {
                    lightIsPresentInCurrentGroup = true;
                }

                break;
            }
        }
    }

    if (_sendFeedback && lightIsPresentInCurrentGroup)
    {
        // Send ID_LIGHTS_UPDATED event to app only if light from the current group has changed
        _fh(FeedbackMessage(FeedbackMessage::REQUEST_TYPE_INTERNAL, FeedbackMessage::ID_LIGHTS_UPDATED, _bridge));
    }
}

void BridgeConfigRetriever::DeleteZigbeeConnectivity(const JSONNode& zc)
{
    std::string id = zc["id"].as_string();

    // Nothing else to do here since deleting the connectivity will also trigger delete of the associated device and light.
    _allZigbeeConnetivityMap.erase(id);
}

void BridgeConfigRetriever::AddEntertainment(const JSONNode& entertainment)
{
    std::string id = entertainment["id"].as_string();

    _allEntertainment[id] = entertainment;
}

void BridgeConfigRetriever::UpdateEntertainment(const JSONNode& entertainment)
{
    // We don't expect any useful properties update for now.
}

void BridgeConfigRetriever::DeleteEntertainment(const JSONNode& entertainment)
{
    std::string id = entertainment["id"].as_string();

    // Nothing else to do here since deleting the entertainment will also trigger delete of the associated light.
    _allEntertainment.erase(id);
}

void BridgeConfigRetriever::AddLight(const JSONNode& light)
{
    std::string id = light["id"].as_string();

    _allLightMap[id] = light;
}

void BridgeConfigRetriever::UpdateLight(const JSONNode& lightUpdateNode)
{
    // Updates only contain a single updated attribute so look for it and update current light json data.
    std::string id = lightUpdateNode["id"].as_string();
    JSONNode curLightNode;

    if (!GetLightNodeById(id, curLightNode))
    {
        HUE_LOG << HUE_CORE << HUE_WARN << "UpdateLight: light not found: " << id << HUE_ENDL;
        return;
    }
    
    std::string newName;
    bool newOnState;
    bool newDynamicEnabled;
    double newBrightness;
    double newXY[2];
    bool onStateUpdate = false;
    bool brightnessUpdate = false;
    bool nameUpdate = false;
    bool colorUpdate = false;
    bool dynamicUpdate = false;

    if (SerializerHelper::IsAttributeSet(&lightUpdateNode, "metadata"))
    {
        auto metadata = lightUpdateNode["metadata"].as_node();

        if (SerializerHelper::IsAttributeSet(&metadata, "name"))
        {
            newName = metadata["name"].as_string();
            curLightNode["metadata"]["name"] = newName;
            nameUpdate = true;
        }
    }
    else if (SerializerHelper::IsAttributeSet(&lightUpdateNode, "on"))
    {
        const JSONNode& onState = lightUpdateNode["on"];

        if (SerializerHelper::IsAttributeSet(&onState, "on"))
        {
            newOnState = onState["on"].as_bool();
            curLightNode["on"]["on"] = newOnState;
            onStateUpdate = true;
        }
    }
    else if (SerializerHelper::IsAttributeSet(&lightUpdateNode, "dimming"))
    {
        const JSONNode& dimmingState = lightUpdateNode["dimming"];

        if (SerializerHelper::IsAttributeSet(&dimmingState, "brightness"))
        {
            newBrightness = dimmingState["brightness"].as_float();
            curLightNode["dimming"]["brightness"] = newBrightness;
            brightnessUpdate = true;
        }
    }
    else if (SerializerHelper::IsAttributeSet(&lightUpdateNode, "color"))
    {
        auto color = lightUpdateNode["color"];

        if (SerializerHelper::IsAttributeSet(&color, "xy"))
        {
            auto xyData = color["xy"].as_node();

            newXY[0] = xyData["x"].as_float();
            newXY[1] = xyData["y"].as_float();

            curLightNode["color"]["xy"]["x"] = newXY[0];
            curLightNode["color"]["xy"]["y"] = newXY[1];
            colorUpdate = true;
        }
    }
    else if (SerializerHelper::IsAttributeSet(&lightUpdateNode, "dynamics"))
    {
        auto dynamicNode = lightUpdateNode["dynamics"];

        if (SerializerHelper::IsAttributeSet(&dynamicNode, "status"))
        {
            auto dynamicStatus = dynamicNode["status"].as_string();
            curLightNode["dynamics"]["status"] = dynamicStatus;
            newDynamicEnabled = dynamicStatus != "none";
            dynamicUpdate = true;
        }
    }

    // Then check if associated to any group in the bridge
    GroupListPtr groupList = _bridge->GetGroups();

    bool lightIsPresentInCurrentGroup = false;

    if (groupList == nullptr)
    {
        // Should not happen
        return;
    }

    bool groupsUpdated = false;

    for (auto groupIt = groupList->begin(); groupIt != groupList->end(); ++groupIt)
    {
        LightListPtr lightList = (*groupIt)->GetPhysicalLights();

        for (int i = 0; i < lightList->size(); i++)
        {
            LightPtr light = lightList->at(i);
            if (light->GetId() == id)
            {
                if (nameUpdate)
                {
                    light->SetName(newName);
                }
                if (onStateUpdate)
                {
                    light->SetOn(newOnState);
                }
                if (brightnessUpdate)
                {
                    light->SetBrightness(newBrightness);
                }
                if (colorUpdate)
                {
                    Color color = light->GetColor();
                    double brightness = light->GetBrightness();
                    light->SetColor({ newXY, brightness, 100.0 });

                    // Make sure that the color was really changed
                    colorUpdate = !(color == light->GetColor());
                }
                if (dynamicUpdate)
                {
                    light->SetDynamicEnabled(newDynamicEnabled);
                }

                if ((*groupIt) == _bridge->GetGroup())
                {
                    lightIsPresentInCurrentGroup = true;
                }

                bool groupUpdated = UpdateGroupOn((*groupIt));
                groupsUpdated = groupsUpdated || groupUpdated;
                groupUpdated = UpdateGroupBrightness((*groupIt));
                groupsUpdated = groupsUpdated || groupUpdated;

                break;
            }
        }

        // Continue the search even if the light was found in a group because it could also exist in other groups
    }

    // Also check if associated to any zone on the bridge
    ZoneListPtr zoneList = _bridge->GetZones();

    for (auto zoneIt = zoneList->begin(); zoneIt != zoneList->end(); ++zoneIt)
    {
        LightListPtr lightList = (*zoneIt)->GetPhysicalLights();

        for (int i = 0; i < lightList->size(); i++)
        {
            LightPtr light = lightList->at(i);
            if (light->GetId() == id)
            {
                const Color& color = light->GetColor();

                if (nameUpdate)
                {
                    light->SetName(newName);
                }
                if (onStateUpdate)
                {
                    light->SetOn(newOnState);
                }
                if (brightnessUpdate)
                {
                    light->SetBrightness(newBrightness);
                }
                if (colorUpdate)
                {
                    double brightness = light->GetBrightness();
                    light->SetColor({ newXY, brightness, 100.0 });
                }
                break;
            }
        }
    }

    if (_sendFeedback && groupsUpdated)
    {
        ScheduleFeedback(FeedbackMessage::ID_GROUP_LIGHTSTATE_UPDATED);
        //_fh(FeedbackMessage(FeedbackMessage::REQUEST_TYPE_INTERNAL, FeedbackMessage::ID_GROUPLIST_UPDATED, _bridge));
    }
    if (_sendFeedback && lightIsPresentInCurrentGroup && !onStateUpdate && !brightnessUpdate && (nameUpdate || colorUpdate || dynamicUpdate))
    {
        // Send ID_LIGHTS_UPDATED event to app only if light from the current group has changed.
        // Also avoid light on state and brightness changed because it's irrelevant unless it impacts the groups
        ScheduleFeedback(FeedbackMessage::ID_LIGHTS_UPDATED);
        //_fh(FeedbackMessage(FeedbackMessage::REQUEST_TYPE_INTERNAL, FeedbackMessage::ID_LIGHTS_UPDATED, _bridge));
    }
}

void BridgeConfigRetriever::DeleteLight(const JSONNode& light)
{
    std::string id = light["id"].as_string();

    // Delete from all list first
    _allLightMap.erase(id);

    // Then check if associated to any group in the bridge
    GroupListPtr groupList = _bridge->GetGroups();
    bool lightWasInGroup = false;

    for (auto group = groupList->begin(); group != groupList->end(); group++)
    {
        LightListPtr lightList = (*group)->GetPhysicalLights();

        for (int i = 0; i < lightList->size(); i++)
        {
            if (lightList->at(i)->GetId() == id)
            {
                // Always create a new light list, otherwise we might have issue if another thread try to access the content of the list while we remove an element from it.
                LightListPtr newLightList = std::make_shared<LightList>();
                newLightList->assign(lightList->begin(), lightList->end());

                newLightList->erase(newLightList->begin() + i);
                (*group)->SetPhysicalLights(newLightList);

                UpdateGroupOn(*group);
                lightWasInGroup = true;
                // Continue the search in this group because if the light is a pls then there's more than one instance of it in the list.
            }
        }

        // Continue the search even if the light was found in a group because it could also exist in other groups
    }

    if (lightWasInGroup)
    {
        ScheduleFeedback(FeedbackMessage::ID_LIGHTS_UPDATED);
    }
}

void BridgeConfigRetriever::AddEntertainmentConfiguration(JSONNode& ec)
{
    // Always create a new group list, otherwise we might have issue if another thread try to access the content of the list while we add a new element in it.
    GroupListPtr groupList = std::make_shared<GroupList>();
    GroupListPtr oldGroupList = _bridge->GetGroups();

    if (oldGroupList != nullptr)
    {
        groupList->assign(oldGroupList->begin(), oldGroupList->end());
    }

    GroupPtr newGroup = ParseEntertainmentConfig(ec);
    groupList->push_back(newGroup);

    _bridge->SetGroups(groupList);

    if (_sendFeedback)
    {
        _fh(FeedbackMessage(FeedbackMessage::REQUEST_TYPE_INTERNAL, FeedbackMessage::ID_GROUPLIST_UPDATED, _bridge));
    }
}

void BridgeConfigRetriever::UpdateEntertainmentConfiguration(JSONNode& ec)
{
    std::string id = ec["id"].as_string();

    GroupPtr group = GetGroupById(id);

    if (group == nullptr)
    {
        // Should not happen
        return;
    }

    bool bridgeWasStreaming = _bridge->IsStreaming();

    bool groupChanged = ParseName(ec, group);
    groupChanged |= ParseClass(ec, group);
    groupChanged |= ParseStreamActive(ec, group);
    groupChanged |= ParseStreamProxy(ec, group);    

    if (_sendFeedback && groupChanged)
    {
        _fh(FeedbackMessage(FeedbackMessage::REQUEST_TYPE_INTERNAL, FeedbackMessage::ID_GROUPLIST_UPDATED, _bridge));
    }

    LightListPtr newLightChannelList = std::make_shared<LightList>();
    GroupChannelToPhysicalLightMapPtr newChannelToPhysicalLightsMap = std::make_shared<GroupChannelToPhysicalLightMap>();
    bool groupLightsChannelChanged = ParseChannels(ec, newLightChannelList, newChannelToPhysicalLightsMap);

    if (groupLightsChannelChanged)
    {
        group->SetLights(newLightChannelList);
        group->SetChannelToPhysicalLightsMap(newChannelToPhysicalLightsMap);
    }

    LightListPtr newPhysicalLightList = std::make_shared<LightList>();
    bool physicalLightsChanged = ParseLights(ec, group, newPhysicalLightList);

    if (physicalLightsChanged)
    {
        group->SetPhysicalLights(newPhysicalLightList);
        group->SetChannelToPhysicalLightsMap(newChannelToPhysicalLightsMap);
    }

    if (_sendFeedback && (groupLightsChannelChanged || physicalLightsChanged))
    {
        _fh(FeedbackMessage(FeedbackMessage::REQUEST_TYPE_INTERNAL, FeedbackMessage::ID_GROUP_LIGHTSTATE_UPDATED, _bridge));
    }
}

void BridgeConfigRetriever::DeleteEntertainmentConfiguration(JSONNode& ec)
{
    std::string id = ec["id"].as_string();

    GroupListPtr groupList = _bridge->GetGroups();

    if (groupList == nullptr)
    {
        // Should not happen
        return;
    }

    for (auto groupIt = groupList->begin(); groupIt != groupList->end(); groupIt++)
    {
        if ((*groupIt)->GetId() != id)
        {
            continue;
        }

        groupList->erase(groupIt);

        break;
    }

    if (_sendFeedback)
    {
        _fh(FeedbackMessage(FeedbackMessage::REQUEST_TYPE_INTERNAL, FeedbackMessage::ID_GROUPLIST_UPDATED, _bridge));
    }

    if (_sendFeedback && _bridge->GetSelectedGroup() == id)
    {
        _bridge->SetSelectedGroup("");
        _fh(FeedbackMessage(FeedbackMessage::REQUEST_TYPE_INTERNAL, FeedbackMessage::ID_SELECT_GROUP, _bridge));
    }
}

void BridgeConfigRetriever::AddZone(JSONNode& zone)
{
    std::string id;
    Serializable::DeserializeValue(&zone, "id", &id, "");

    if (id.empty())
    {
        return;
    }

    // Always create a new zone list, otherwise we might have issue if another thread try to access the content of the list while we add a new element in it.
    ZoneListPtr zoneList = std::make_shared<ZoneList>();
    ZoneListPtr oldZoneList = _bridge->GetZones();

    if (oldZoneList != nullptr)
    {
        zoneList->assign(oldZoneList->begin(), oldZoneList->end());
    }

    ZonePtr newZone = std::make_shared<Zone>();

    newZone->SetId(id);

    std::string idv1 = GetIdV1(zone);
    newZone->SetIdV1(idv1);

    // Metadata
    if (SerializerHelper::IsAttributeSet(&zone, "metadata"))
    {
        std::string name;
        JSONNode metadata = zone["metadata"];
        Serializable::DeserializeValue(&metadata, "name", &name, "");

        newZone->SetName(name);

        std::string archetype;
        Serializable::DeserializeValue(&metadata, "archetype", &archetype, "");

        if (archetype == "computer")
        {
            newZone->SetArchetype(ZONEARCHETYPE_COMPUTER);
        }
        else
        {
            newZone->SetArchetype(ZONEARCHETYPE_OTHER);
        }
    }

    // Find and set grouped_light id. We check the children attribute first because things have changed since first implementation and if
    // children exist then grouped_light is in services instead of in grouped_services.
    JSONNode servicesArr(JSON_NULL);
    if (SerializerHelper::IsAttributeSet(&zone, "children"))
    {
        servicesArr = SerializerHelper::GetAttributeValue(&zone, "services");
    }
    else if (SerializerHelper::IsAttributeSet(&zone, "grouped_services"))
    {
        servicesArr = SerializerHelper::GetAttributeValue(&zone, "grouped_services");
    }

    if (servicesArr.type() != JSON_NULL)
    {
        auto groupedServices = servicesArr.as_array();

        for (json_index_t i = 0; i < groupedServices.size(); i++)
        {
            JSONNode gropuService = groupedServices[i].as_node();
            if (gropuService["rtype"] == "grouped_light")
            {
                newZone->SetGroupedLightId(gropuService["rid"].as_string());
                break;
            }
        }
    }

    // Lights
    LightListPtr lightList = std::make_shared<LightList>();
    ParseZoneLights(zone, lightList);

    newZone->SetPhysicalLights(lightList);

    zoneList->push_back(newZone);

    _bridge->SetZones(zoneList);

    if (_sendFeedback)
    {
        _fh(FeedbackMessage(FeedbackMessage::REQUEST_TYPE_INTERNAL, FeedbackMessage::ID_ZONELIST_UPDATED, _bridge));
    }

    // TODO not sure we need that map
    _allZoneMap[id] = zone;
}

void BridgeConfigRetriever::UpdateZone(JSONNode& zone)
{
    std::string id;
    Serializable::DeserializeValue(&zone, "id", &id, "");

    if (id.empty())
    {
        return;
    }

    ZonePtr zoneObj = GetZoneById(id);

    if (zoneObj == nullptr)
    {
        return;
    }

    bool updated = false;

    if (SerializerHelper::IsAttributeSet(&zone, "metadata"))
    {
        JSONNode metadata = zone["metadata"];

        if (SerializerHelper::IsAttributeSet(&metadata, "name"))
        {
            std::string name;
            Serializable::DeserializeValue(&metadata, "name", &name, "");
            std::string oldName = zoneObj->GetName();
            zoneObj->SetName(name);
            updated = oldName != name;
        }

        if (SerializerHelper::IsAttributeSet(&metadata, "archetype"))
        {
            std::string archetype;
            Serializable::DeserializeValue(&metadata, "archetype", &archetype, "");

            std::transform(archetype.begin(), archetype.end(), archetype.begin(), [](unsigned char c)
            {
                return std::tolower(c);
            });

            ZoneArchetype oldArchetype = zoneObj->GetArchetype();

            if (archetype == "computer")
            {
                zoneObj->SetArchetype(ZONEARCHETYPE_COMPUTER);
            }
            else
            {
                zoneObj->SetArchetype(ZONEARCHETYPE_OTHER);
            }

            updated = oldArchetype != zoneObj->GetArchetype();
        }
    }

    // Parse the lights list
    LightListPtr lightList = std::make_shared<LightList>();
    if (ParseZoneLights(zone, lightList))
    {
        zoneObj->SetPhysicalLights(lightList);
        updated = true;
    }

    if (_sendFeedback && updated)
    {
        _fh(FeedbackMessage(FeedbackMessage::REQUEST_TYPE_INTERNAL, FeedbackMessage::ID_ZONELIST_UPDATED, _bridge));
    }
}

void BridgeConfigRetriever::DeleteZone(JSONNode& zone)
{
    std::string id = zone["id"].as_string();

    _allZoneMap.erase(id);

    ZoneListPtr zoneList = _bridge->GetZones();

    for (auto zoneIt = zoneList->begin(); zoneIt != zoneList->end(); ++zoneIt)
    {
        if ((*zoneIt)->GetId() != id)
        {
            continue;
        }

        zoneList->erase(zoneIt);

        break;
    }

    if (_sendFeedback)
    {
        _fh(FeedbackMessage(FeedbackMessage::REQUEST_TYPE_INTERNAL, FeedbackMessage::ID_ZONELIST_UPDATED, _bridge));
    }
}

void BridgeConfigRetriever::AddGroupedLight(JSONNode& groupedLight)
{
    ParseGroupedLight(groupedLight);
}

void BridgeConfigRetriever::UpdateGroupedLight(JSONNode& groupedLight)
{
    bool updated = ParseGroupedLight(groupedLight);

    if (_sendFeedback && updated)
    {
        _fh(FeedbackMessage(FeedbackMessage::REQUEST_TYPE_INTERNAL, FeedbackMessage::ID_GROUPLIST_UPDATED, _bridge));
    }
}

void BridgeConfigRetriever::AddBridge(JSONNode& bs)
{
    _bridgeJsonNode = bs;

    std::string deviceId = GetDeviceIdFromServiceReferenceId(bs["id"].as_string());

    if (deviceId.empty())
    {
        return;
    }

    JSONNode device = _allDeviceMap[deviceId];

    std::string name;
    if (SerializerHelper::IsAttributeSet(&device, "metadata"))
    {
        JSONNode metadata = device["metadata"];
        Serializable::DeserializeValue(&metadata, "name", &name, "");

        _bridge->SetName(name);
    }

    std::string modelId;
    if (SerializerHelper::IsAttributeSet(&device, "product_data"))
    {
        JSONNode productData = device["product_data"];
        Serializable::DeserializeValue(&productData, "model_id", &modelId, "");
    }

    _bridge->SetModelId(modelId);

    // TODO
    /*if (SerializerHelper::IsAttributeSet(&bs, "apis"))
    {
        JSONNode api = bs["apis"];

        if (SerializerHelper::IsAttributeSet(&api, "hue_streaming"))
        {
            std::string apiVersion;
            JSONNode apiStreaming = api["hue_streaming"];
            Serializable::DeserializeValue(&apiStreaming, "version", &apiVersion, "");
            _bridge->SetApiversion(apiVersion);
        }
    }*/

    std::string bridgeId;
    Serializable::DeserializeValue(&bs, "bridge_id", &bridgeId, "");

    // Convert id to upper case to keep compatibility with clipv1
    std::transform(bridgeId.begin(), bridgeId.end(), bridgeId.begin(), [](unsigned char c)
    {
        return std::toupper(c);
    });

    _bridge->SetId(bridgeId);

    // Fetch maximum number of parallel streaming sessions, otherwise default to 1.
    int maxNoStreamingSession = 1;
    JSONNode entertainmentService;
    if (GetServiceByType("entertainment", device, entertainmentService))
    {
        if (SerializerHelper::IsAttributeSet(&entertainmentService, "max_streams"))
        {
            maxNoStreamingSession = static_cast<int>(entertainmentService["max_streams"].as_int());
        }
    }

    _bridge->SetMaxNoStreamingSessions(maxNoStreamingSession);
}

void BridgeConfigRetriever::UpdateBridge(JSONNode& bs)
{
    // The only interesting property that could change is the api version

    // TODO
    /*if (SerializerHelper::IsAttributeSet(&bs, "apis"))
    {
        JSONNode api = bs["apis"];

        if (SerializerHelper::IsAttributeSet(&api, "hue_streaming"))
        {
            std::string apiVersion;

            JSONNode apiStreaming = api["hue_streaming"];
            Serializable::DeserializeValue(&apiStreaming, "version", &apiVersion, "");

            _bridgeJsonNode["api"]["hue_streaming"]["version"] = apiVersion;
            _bridge->SetApiversion(apiVersion);
        }
    }*/
}

void BridgeConfigRetriever::DeleteBridge(JSONNode& bs)
{
    // This should not happen
}

void BridgeConfigRetriever::AddScene(JSONNode& scene)
{
    // First make sure that's a scene linked to a zone
    ZonePtr zone = GetZoneFromScene(scene);

    if (zone == nullptr)
    {
        return;
    }

    ScenePtr newScene = nullptr;
    ParseScene(scene, newScene);

    if (newScene != nullptr)
    {
        zone->GetScenes()->push_back(newScene);

        if (_sendFeedback)
        {
            _fh(FeedbackMessage(FeedbackMessage::REQUEST_TYPE_INTERNAL, FeedbackMessage::ID_ZONE_SCENELIST_UPDATED, _bridge));
        }

        _allSceneMap[newScene->GetId()] = scene;
    }
}

void BridgeConfigRetriever::UpdateScene(JSONNode& scene)
{
    // First make sure that's a scene linked to a zone
    ZonePtr zone = GetZoneFromScene(scene);

    if (zone == nullptr)
    {
        return;
    }

    SceneListPtr sceneList = zone->GetScenes();

    std::string id;
    Serializable::DeserializeValue(&scene, "id", &id, "");

    auto sceneIt = std::find_if(sceneList->begin(), sceneList->end(), [&](const ScenePtr aScene)
    {
        return aScene->GetId() == id;
    });

    if (sceneIt != sceneList->end())
    {
        if (ParseScene(scene, *sceneIt) && _sendFeedback)
        {
            ScheduleFeedback(FeedbackMessage::ID_ZONE_SCENELIST_UPDATED);
        }
    }
}

void BridgeConfigRetriever::DeleteScene(JSONNode& scene)
{
    ZonePtr zone = GetZoneFromScene(scene);

    if (zone == nullptr)
    {
        return;
    }

    SceneListPtr sceneList = zone->GetScenes();

    std::string id;
    Serializable::DeserializeValue(&scene, "id", &id, "");

    _allSceneMap.erase(id);

    auto sceneIt = std::find_if(sceneList->begin(), sceneList->end(), [&](const ScenePtr aScene)
    {
        return aScene->GetId() == id;
    });

    if (sceneIt != sceneList->end())
    {
        sceneList->erase(sceneIt);

        if (_sendFeedback)
        {
            _fh(FeedbackMessage(FeedbackMessage::REQUEST_TYPE_INTERNAL, FeedbackMessage::ID_ZONE_SCENELIST_UPDATED, _bridge));
        }
    }
}

std::string BridgeConfigRetriever::GetLightIdFromZigbeeConnectivityId(const std::string& zcId)
{
    // Find the device associated with this Zigbee connectivity id first
    std::string deviceId = GetDeviceIdFromServiceReferenceId(zcId);

    if (deviceId.empty())
    {
        return "";
    }

    return GetDeviceReferenceIdFromReferenceType(deviceId, "light");
}

std::string BridgeConfigRetriever::GetLightIdFromEntertainmentId(const std::string& entertainmentId)
{
    // Find the device associated with this entertainment id first
    std::string deviceId = GetDeviceIdFromServiceReferenceId(entertainmentId);

    if (deviceId.empty())
    {
        return "";
    }

    return GetDeviceReferenceIdFromReferenceType(deviceId, "light");
}

std::string BridgeConfigRetriever::GetZigbeeConnectivityIdFromLightId(const std::string& lightId)
{
    // Find the device associated with this light id first
    std::string deviceId = GetDeviceIdFromServiceReferenceId(lightId);

    if (deviceId.empty())
    {
        return "";
    }

    return GetDeviceReferenceIdFromReferenceType(deviceId, "zigbee_connectivity");
}

std::string BridgeConfigRetriever::GetDeviceIdFromServiceReferenceId(const std::string& serviceReferenceId) const
{
    for (auto deviceIt = _allDeviceMap.begin(); deviceIt != _allDeviceMap.end(); deviceIt++)
    {
        auto serviceArr = deviceIt->second["services"].as_array();

        for (json_index_t i = 0; i < serviceArr.size(); i++)
        {
            if (serviceArr[i].as_node()["rid"] == serviceReferenceId)
            {
                return deviceIt->first;
            }
        }
    }

    return "";
}

std::string BridgeConfigRetriever::GetDeviceReferenceIdFromReferenceType(const std::string& deviceId, const std::string& referenceType)
{
    if (deviceId.empty())
    {
        return "";
    }

    auto serviceArr = _allDeviceMap[deviceId]["services"].as_array();

    for (json_index_t i = 0; i < serviceArr.size(); i++)
    {
        JSONNode service = serviceArr[i].as_node();
        if (service["rtype"] == referenceType)
        {
            return service["rid"].as_string();
        }
    }

    return "";
}

bool BridgeConfigRetriever::GetServiceByType(const std::string& rtype, const JSONNode& node, JSONNode& service)
{
    if (!SerializerHelper::IsAttributeSet(&node, "services"))
    {
        return false;
    }

    JSONNode services = SerializerHelper::GetAttributeValue(&node, "services");

    if (services.type() != JSON_ARRAY)
    {
        return false;
    }

    auto serviceArr = services.as_array();

    for (json_index_t i = 0; i < serviceArr.size(); i++)
    {
        if (serviceArr[i].as_node()["rtype"] == rtype)
        {
            JSONNode s = serviceArr[i].as_node();

            auto entertainmentIt = _allEntertainment.find(s["rid"].as_string());

            if (entertainmentIt != _allEntertainment.end())
            {
                service = entertainmentIt->second;
            }

            return true;
        }
    }

    return false;
}

GroupPtr BridgeConfigRetriever::GetGroupById(const std::string& id) const
{
    GroupListPtr groupList = _bridge->GetGroups();

    if (groupList == nullptr)
    {
        // Should not happen
        return nullptr;
    }

    for (auto groupIt = groupList->begin(); groupIt != groupList->end(); ++groupIt)
    {
        if ((*groupIt)->GetId() == id)
        {
            return (*groupIt);
        }
    }

    return nullptr;
}

GroupPtr BridgeConfigRetriever::GetGroupByIdV1(const std::string& id) const
{
    // Entertainment configuration id_v1 is not supported anymore by the bridge
    return nullptr;
}

GroupPtr BridgeConfigRetriever::GetGroupByName(const std::string& name) const
{
    GroupListPtr groupList = _bridge->GetGroups();

    if (groupList == nullptr)
    {
        // Should not happen
        return nullptr;
    }

    for (auto groupIt = groupList->begin(); groupIt != groupList->end(); ++groupIt)
    {
        if ((*groupIt)->GetName() == name)
        {
            return (*groupIt);
        }
    }

    return nullptr;
}

ZonePtr BridgeConfigRetriever::GetZoneFromScene(JSONNode& scene)
{
    // First make sure that's a scene linked to a zone
    JSONNode groupNode = SerializerHelper::GetAttributeValue(&scene, "group");
    JSONNode refTypeNode = SerializerHelper::GetAttributeValue(&groupNode, "rtype");

    // If scene node doesn't contains a group attribute then check for it in a previously saved scene node instead
    if (refTypeNode.type() != JSON_STRING)
    {
        JSONNode id = SerializerHelper::GetAttributeValue(&scene, "id");

        if (id.type() != JSON_STRING)
        {
            return nullptr;
        }

        auto sceneIt = _allSceneMap.find(id.as_string());

        if (sceneIt != _allSceneMap.end())
        {
            groupNode = SerializerHelper::GetAttributeValue(&(sceneIt->second), "group");
            refTypeNode = SerializerHelper::GetAttributeValue(&groupNode, "rtype");
        }
    }

    if (refTypeNode.type() != JSON_STRING || refTypeNode.as_string() != "zone" || !SerializerHelper::IsAttributeSet(&groupNode, "rid"))
    {
        return nullptr;
    }

    // Now find the zone associated with this scene
    std::string zoneId = groupNode["rid"].as_string();

    ZoneListPtr zoneList = _bridge->GetZones();
    ZonePtr zone = nullptr;

    for (auto zoneIt = zoneList->begin(); zoneIt != zoneList->end(); ++zoneIt)
    {
        if ((*zoneIt)->GetId() == zoneId)
        {
            return *zoneIt;
        }
    }

    return nullptr;
}

ZonePtr BridgeConfigRetriever::GetZoneById(const std::string& id)
{
    ZoneListPtr zoneList = _bridge->GetZones();

    if (zoneList == nullptr)
    {
        // Should not happen
        return nullptr;
    }

    for (auto zoneIt = zoneList->begin(); zoneIt != zoneList->end(); ++zoneIt)
    {
        if ((*zoneIt)->GetId() == id)
        {
            return (*zoneIt);
        }
    }

    return nullptr;
}

bool BridgeConfigRetriever::UpdateGroupOn(GroupPtr group)
{
    // TODO Remove this function when zones become available
    LightListPtr lightList = group->GetPhysicalLights();

    bool on = false;
    for (auto lightIt = lightList->begin(); lightIt != lightList->end(); ++lightIt)
    {
        if ((*lightIt)->Reachable())
        {
            on = on || (*lightIt)->On();
        }
    }

    bool updated = group->OnState() != on;
    group->SetOnState(on);

    return updated;
}

bool BridgeConfigRetriever::UpdateGroupBrightness(GroupPtr group)
{
    // TODO Remove this function when zones become available
    LightListPtr lightList = group->GetPhysicalLights();

    double averageBrightness = 0.0;
    uint32_t numReachableLights = 0;

    if (lightList->size() > 0)
    {
        for (auto lightIt = lightList->begin(); lightIt != lightList->end(); ++lightIt)
        {
            if ((*lightIt)->Reachable())
            {
                averageBrightness += (*lightIt)->GetBrightness();
                numReachableLights++;
            }
        }

        averageBrightness /= (numReachableLights > 0 ? numReachableLights : 1);
    }

    bool updated = std::abs((group->GetBrightnessState() * 100.0) - averageBrightness) > 0.1;
    group->SetBrightnessState(averageBrightness /= 100.0);

    return updated;
}

bool BridgeConfigRetriever::GetLightNodeById(const string& id, JSONNode& lightNode)
{
    std::unordered_map<std::string, JSONNode>::iterator it = _allLightMap.find(id);

    if (it == _allLightMap.end())
    {
        HUE_LOG << HUE_CORE << HUE_WARN << "GetLightNodeById: light not found: " << id << HUE_ENDL;
        return false;
    }

    lightNode = it->second;
    return true;
}
