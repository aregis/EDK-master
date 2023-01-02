/*******************************************************************************
Copyright (C) 2020 Signify Holding
All Rights Reserved.
********************************************************************************/

#ifndef HUESTREAM_CONNECT_BridgeConfigRetriever_H_
#define HUESTREAM_CONNECT_BridgeConfigRetriever_H_

#include <memory>
#include <string>
#include <unordered_map>
#include "huestream/connect/IFullConfigRetriever.h"
#include "huestream/connect/FeedbackMessage.h"
#include "huestream/common/data/Bridge.h"
#include "libjson/libjson.h"
#include "huestream/common/http/IBridgeHttpClient.h"
#include "support/network/http/HttpRequest.h"
#include "support/threading/SynchronousExecutor.h"

namespace huestream
{
	class BridgeConfigRetriever: public IConfigRetriever, public std::enable_shared_from_this<BridgeConfigRetriever>
	{
	public:
		explicit BridgeConfigRetriever(const BridgeHttpClientPtr http, bool useForcedActivation = true);
		~BridgeConfigRetriever();

		bool Execute(BridgePtr bridge, RetrieveCallbackHandler cb, FeedbackHandler fh) override;
		void OnBridgeMonitorEvent(const FeedbackMessage& message) override;
		bool IsSupportingClipV2() override {return true;};
		void RefreshBridgeConnection() override;

	private:
		typedef std::vector<std::tuple<std::string, std::function<void(JSONNode&)>>> ResourceInfo;

		bool DoExecute(BridgePtr bridge, RetrieveCallbackHandler cb, FeedbackHandler fh);
		void Abort();
		void OnBridgeDisconnect();
		void GetFullConfig();
		void GetResource(const std::string& cursor);
		void OnGetResource();
		void UpdateWhitelist(std::function<void()> callback);
		void GetApplicationId(std::function<void()> callback);
		void ListenToEvents();
		void ParseEventResponseAndExecuteCallback();
		bool ParseJsonEvent(JSONNode& root);
		GroupPtr ParseEntertainmentConfig(const JSONNode &node);
		bool ParseLights(const JSONNode &node, GroupPtr group, LightListPtr lightList);
		bool ParseChannels(const JSONNode &node, LightListPtr lightList, GroupChannelToPhysicalLightMapPtr channelToPhysicalLightsMap);
		bool ParseClass(const JSONNode& node, GroupPtr group);
		bool ParseName(const JSONNode& node, GroupPtr group);
		bool ParseStreamActive(const JSONNode& node, GroupPtr group);
		bool ParseStreamProxy(const JSONNode& node, GroupPtr group);
		LightPtr ParseLightInfo(const JSONNode& node);
		bool ParseGroupedLight(const JSONNode& node);
		bool ParseScene(const JSONNode& node, ScenePtr& scene);
		bool ParseZoneLights(const JSONNode &node, LightListPtr lightList = nullptr);
		std::string GetOwnerName(const std::string& userName, const std::string& ecId);
		std::string GetIdV1(const JSONNode& node) const;
		void Finish(OperationResult result, bool unBusy = true);
		void ScheduleFeedback(FeedbackMessage::Id);
		void DispatchFeedback();
		bool ValidateHttpRequestStatus(const support::HttpRequestError& error, const support::IHttpResponse& response);

		void AddDevice(JSONNode& device);
		void UpdateDevice(JSONNode& device);
		void DeleteDevice(JSONNode& device);
		void AddZigbeeConnectivity(const JSONNode& zc);
		void UpdateZigbeeConnectivity(const JSONNode& zc);
		void DeleteZigbeeConnectivity(const JSONNode& zc);
		void AddEntertainment(const JSONNode& entertainment);
		void UpdateEntertainment(const JSONNode& entertainment);
		void DeleteEntertainment(const JSONNode& entertainment);
		void AddLight(const JSONNode& light);
		void UpdateLight(const JSONNode& lightUpdateNode);
		void DeleteLight(const JSONNode& light);
		void AddEntertainmentConfiguration(JSONNode& ec);
		void UpdateEntertainmentConfiguration(JSONNode& ec);
		void DeleteEntertainmentConfiguration(JSONNode& ec);
		void AddZone(JSONNode& zone);
		void UpdateZone(JSONNode& zone);
		void DeleteZone(JSONNode& zone);
		void AddGroupedLight(JSONNode& groupedLight);
		void UpdateGroupedLight(JSONNode& groupedLight);
		void AddBridge(JSONNode& bs);
		void UpdateBridge(JSONNode& bs);
		void DeleteBridge(JSONNode& bs);
		void AddScene(JSONNode& scene);
		void UpdateScene(JSONNode& scene);
		void DeleteScene(JSONNode& scene);

		std::string GetZigbeeConnectivityIdFromLightId(const std::string& lightId);
		std::string GetLightIdFromZigbeeConnectivityId(const std::string& zcId);
		std::string GetLightIdFromEntertainmentId(const std::string& entertainmentId);
		std::string GetDeviceIdFromServiceReferenceId(const std::string& serviceReferenceId) const;
		std::string GetDeviceReferenceIdFromReferenceType(const std::string& deviceId, const std::string& referenceType);
		bool GetServiceByType(const std::string& rtype, const JSONNode& node, JSONNode& service);
		GroupPtr GetGroupById(const std::string& id) const;
		GroupPtr GetGroupByIdV1(const std::string& id) const;
		GroupPtr GetGroupByName(const std::string& name) const;
		ZonePtr GetZoneFromScene(JSONNode& scene);
		ZonePtr GetZoneById(const std::string& id);
		bool UpdateGroupOn(GroupPtr group);
		bool UpdateGroupBrightness(GroupPtr group);

		bool GetLightNodeById(const string& id, JSONNode& lightNode);

		BridgeHttpClientPtr _http;
		bool _useForcedActivation;
		RetrieveCallbackHandler _cb;
		FeedbackHandler _fh;
		std::atomic<bool> _connected;
		std::atomic<bool> _refreshing;
		std::atomic<bool> _busy;
		bool _sendFeedback;
		std::string _response;
		std::string _responseEvent;
		BridgePtr _bridge;
		JSONNode _whitelist;
		std::unordered_map<string, JSONNode> _allLightMap;
		std::unordered_map<string, JSONNode> _allZigbeeConnetivityMap;
		std::unordered_map<string, JSONNode> _allDeviceMap;
		std::unordered_map<string, JSONNode> _allZoneMap;
		std::unordered_map<string, JSONNode> _allEntertainment;
		std::unordered_map<string, JSONNode> _allSceneMap;
		JSONNode _bridgeJsonNode;
		int32_t _eventRequestId;
		ResourceInfo _resourceInfoList;
		ResourceInfo::iterator _resourceToGet;
		std::unordered_map<int, bool> _feedbackMessageMap;
		std::unique_ptr<support::SynchronousExecutor> _executor;
		bool _nextSendFeedbackIsScheduled;
		std::mutex _feedbackLock;
		std::chrono::steady_clock::time_point _lastFeedbackTime;
		bool _canSendStreamingDisconnectedEvent;
		bool _sendConnectFeedback;
		std::string _previouslySelectedGroupName;
	};
}  // namespace huestream

#endif  // HUESTREAM_CONNECT_BridgeConfigRetriever_H_
