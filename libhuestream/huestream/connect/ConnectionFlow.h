/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#ifndef HUESTREAM_CONNECT_CONNECTIONFLOW_H_
#define HUESTREAM_CONNECT_CONNECTIONFLOW_H_

#include "huestream/common/data/Bridge.h"
#include "huestream/stream/IStream.h"
#include "huestream/connect/FeedbackMessage.h"
#include "huestream/connect/IConnectionFlowFactory.h"
#include "huestream/connect/IConnectionFlow.h"
#include "huestream/connect/IBridgeSearcher.h"
#include "support/scheduler/Scheduler.h"

#include <memory>
#include <string>
#include <functional>
#include <map>

namespace huestream {

class ConnectionFlow: public IConnectionFlow, public std::enable_shared_from_this<ConnectionFlow> {
 public:
    ConnectionFlow(ConnectionFlowFactoryPtr factory, StreamPtr stream, BridgeSettingsPtr bridgeSettings, AppSettingsPtr appSettings, BridgeStorageAccessorPtr storageAccessor);

    ~ConnectionFlow() override;

    void Load() override;

    void ConnectToBridge() override;

    void ConnectToBridgeBackground() override;

    void ConnectToBridgeWithIp(const std::string &ipAddress) override;

    void ConnectToNewBridge() override;

    void SetManual(BridgePtr bridge) override;

    void ConnectoToBridgeWithIdKey(const std::string& id, const std::string& user, const std::string& clientKey) override;

    void ResetBridge() override;

    void ResetAll() override;

    void SelectGroup(std::string id) override;

    void StartStream(StreamPtr stream) override;

    void StopStream(StreamPtr stream) override;

    void Abort() override;

    void OnBridgeMonitorEvent(const FeedbackMessage &message) override;

    void NewMessage(const FeedbackMessage &message) override;

    void SetFeedbackMessageCallback(FeedbackMessageCallback callback) override;
    ConnectionFlowState GetState() override;

 protected:
    struct AuthenticationProcessInfo {
        AuthenticationProcessInfo() : AuthenticationProcessInfo(-1) {}
        explicit AuthenticationProcessInfo(int _bridgeNumber) : failedTries(0), bridgeNumber(_bridgeNumber) {}
        int failedTries;
        int bridgeNumber;
    };

    void DoLoad();

    void DoConnectToBridge();

    void DoConnectToBridgeBackground();

    void DoConnectToBridgeWithIp(const std::string &ipAddress);

    void DoConnectToNewBridge();

    void DoSetManual(BridgePtr bridge);

    void DoConnectoToBridgeWithIdKey(const std::string& id, const std::string& user, const std::string& clientKey);

    void DoReset(bool onlyActiveBridge);

    void DoSelectGroup(std::string id);

    void DoStartStream();

    void DoStopStream();

    void DoAbort();

    void DoOnBridgeMonitorEvent(const FeedbackMessage &message);

    bool Start(FeedbackMessage::RequestType request);

    void StartLoading(std::function<void()> callback);

    void LoadingCompleted(OperationResult result, HueStreamDataPtr persistentData, std::function<void()> callback);

    void StartBridgeSearch();

    void BridgeSearchCompleted(BridgeListPtr bridges);

    BridgeListPtr FilterInvalidAndDuplicateIpAddresses(BridgeListPtr bridges);

    bool ContainsValidBridge(BridgeListPtr bridges);

    void StartAuthentication(BridgePtr bridge);

    void StartAuthentication(BridgeListPtr bridges);

    void AuthenticationCompleted(BridgePtr bridge);

    void PushLinkBridge(BridgePtr bridge);

    void StartRetrieveSmallConfig(BridgePtr bridge);

    void RetrieveSmallConfigCompleted(OperationResult result, BridgePtr bridge);

    void StartRetrieveFullConfig();

    void RetrieveFullConfigCompleted(OperationResult result, BridgePtr bridge);

    void RetrieveFailed(BridgePtr bridge);

    void ReportActionRequired();

    void ActivateStreaming();

    void DeactivateStreaming();

    void ClearBridge();

    void StartSaving();

    void SavingCompleted(OperationResult r);

    void Finish();

    void SchedulerPushlinkTimedOut(BridgePtr bridge);

    ConnectionFlowFactoryPtr _factory;
    AppSettingsPtr _appSettings;
    BridgeSettingsPtr _bridgeSettings;
    BridgeStorageAccessorPtr _storageAccessor;
    BridgeSearcherPtr _bridgeSearcher;
    ConfigRetrieverPtr _smallConfigRetriever;
    ConfigRetrieverPtr _fullConfigRetriever;
    ConnectionFlowState _state;
    FeedbackMessage::RequestType _request;
    size_t _ongoingAuthenticationCount;
    BridgeListPtr _backgroundDiscoveredBridges;
    StreamPtr _stream;
    FeedbackMessageCallback _feedbackMessageCallback;
    HueStreamDataPtr _persistentData;
    BridgePtr _bridgeStartState;
    std::map<BridgePtr, AuthenticationProcessInfo> _authenticationProcessesInfo;
    std::unique_ptr<support::Scheduler> _scheduler;
};
}  // namespace huestream

#endif  // HUESTREAM_CONNECT_CONNECTIONFLOW_H_
