/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/


#include <huestream/connect/ConnectionFlow.h>

#include <algorithm>
#include <memory>
#include <string>

#include "huestream/connect/BridgeStreamingChecker.h"
#include "huestream/config/Config.h"
#include "support/scheduler/SchedulerFactory.h"

constexpr static auto SCHEDULER_INTERVAL_MS = 250;
constexpr static auto PUSHLINK_RETRY_INTERVAL_MS = 1000;
constexpr static auto PUSHLINK_INVALID_IP_MAX_RETRIES = 3;

namespace huestream {

#define DISPATCH_P0(method) \
  MESSAGE_DISPATCH_P0(_factory->GetMessageDispatcher(), ConnectionFlow::method);

#define DISPATCH_P1(method, value) \
  MESSAGE_DISPATCH_P1(_factory->GetMessageDispatcher(), ConnectionFlow::method, value);

#define DISPATCH_P2(method, value1, value2) \
  MESSAGE_DISPATCH_P2(_factory->GetMessageDispatcher(), ConnectionFlow::method, value1, value2);

#define DISPATCH_P3(method, value1, value2, value3) \
  MESSAGE_DISPATCH_P3(_factory->GetMessageDispatcher(), ConnectionFlow::method, value1, value2, value3);

ConnectionFlow::ConnectionFlow(ConnectionFlowFactoryPtr factory, StreamPtr stream, BridgeSettingsPtr bridgeSettings, AppSettingsPtr appSettings, BridgeStorageAccessorPtr storageAccessor) :
    _appSettings(std::move(appSettings)),
    _bridgeSettings(bridgeSettings),
    _storageAccessor(std::move(storageAccessor)),
    _state(Idle),
    _request(FeedbackMessage::REQUEST_TYPE_INTERNAL),
    _ongoingAuthenticationCount(0),
    _backgroundDiscoveredBridges(std::make_shared<BridgeList>()),
    _stream(std::move(stream)),
    _persistentData(std::make_shared<HueStreamData>(bridgeSettings)),
    _bridgeStartState(std::make_shared<Bridge>(bridgeSettings)),
    _scheduler(support::SchedulerFactory::create(SCHEDULER_INTERVAL_MS)) {
    _factory = std::move(factory);
    _persistentData->Clear();
    _backgroundDiscoveredBridges->clear();
}

ConnectionFlow::~ConnectionFlow() {
}

void ConnectionFlow::Load() {
    DISPATCH_P0(DoLoad);
}

void ConnectionFlow::DoLoad() {
    if (!Start(FeedbackMessage::REQUEST_TYPE_LOAD_BRIDGE))
        return;

    StartLoading([this](){
        Finish();
    });
}

void ConnectionFlow::ConnectToBridge() {
    DISPATCH_P0(DoConnectToBridge);
}

void ConnectionFlow::DoConnectToBridge() {
    if (!Start(FeedbackMessage::REQUEST_TYPE_CONNECT))
        return;

    if (!_backgroundDiscoveredBridges->empty()) {
        auto bridges = _backgroundDiscoveredBridges;
        _backgroundDiscoveredBridges = std::make_shared<BridgeList>();
        _backgroundDiscoveredBridges->clear();
        StartAuthentication(bridges);
    } else {
        StartLoading([this](){
            if (_persistentData->GetActiveBridge()->IsEmpty()) {
                StartBridgeSearch();
            } else {
                StartRetrieveSmallConfig(_persistentData->GetActiveBridge());
            }
        });
    }
}

void ConnectionFlow::ConnectToBridgeBackground() {
    DISPATCH_P0(DoConnectToBridgeBackground);
}

void ConnectionFlow::DoConnectToBridgeBackground() {
    if (!Start(FeedbackMessage::REQUEST_TYPE_CONNECT_BACKGROUND))
        return;

    StartLoading([this](){
        if (_persistentData->GetActiveBridge()->IsEmpty()) {
            StartBridgeSearch();
        } else {
            StartRetrieveSmallConfig(_persistentData->GetActiveBridge());
        }
    });
}

void ConnectionFlow::ConnectToBridgeWithIp(const std::string &ipAddress) {
    DISPATCH_P1(DoConnectToBridgeWithIp, ipAddress);
}

void ConnectionFlow::DoConnectToBridgeWithIp(const std::string &ipAddress) {
    if (!Start(FeedbackMessage::REQUEST_TYPE_CONNECT_IP))
        return;

    auto bridge = std::make_shared<Bridge>(_bridgeSettings);
    bridge->SetIpAddress(ipAddress);
    bridge->SetIsValidIp(true);

    StartLoading([this, bridge](){
        StartRetrieveSmallConfig(bridge);
    });
}

void ConnectionFlow::ConnectToNewBridge() {
    DISPATCH_P0(DoConnectToNewBridge);
}

void ConnectionFlow::DoConnectToNewBridge() {
    if (!Start(FeedbackMessage::REQUEST_TYPE_CONNECT_NEW))
        return;

    if (_persistentData->GetActiveBridge()->IsStreaming()) {
        _stream->Stop(_persistentData->GetActiveBridge());
    }

    _persistentData->SetActiveBridge(std::make_shared<Bridge>(_bridgeSettings));

    StartLoading([this]() {
        StartBridgeSearch();
    });
}

void ConnectionFlow::SetManual(BridgePtr bridge) {
    DISPATCH_P1(DoSetManual, bridge);
}

void ConnectionFlow::DoSetManual(BridgePtr bridge) {
    if (!Start(FeedbackMessage::REQUEST_TYPE_SET_MANUAL_BRIDGE))
        return;

    if (_persistentData->GetActiveBridge()->IsStreaming()) {
        _stream->Stop(_persistentData->GetActiveBridge());
    }

    bridge->SetIsValidIp(true);
    bridge->SetIsAuthorized(true);

    StartLoading([this, bridge](){
        StartRetrieveSmallConfig(bridge);
    });
}

void ConnectionFlow::ConnectoToBridgeWithIdKey(const std::string& id, const std::string& user, const std::string& clientKey) {
  DISPATCH_P3(DoConnectoToBridgeWithIdKey, id, user, clientKey);
}

void ConnectionFlow::DoConnectoToBridgeWithIdKey(const std::string& id, const std::string& user, const std::string& clientKey) {
  if (!Start(FeedbackMessage::REQUEST_TYPE_CONNECT_ID_KEY))
    return;

  if (_persistentData->GetActiveBridge()->IsStreaming()) {
    _stream->Stop(_persistentData->GetActiveBridge());
  }

  auto bridge = std::make_shared<Bridge>(_bridgeSettings);
  bridge->SetId(id);
  bridge->SetUser(user);
  bridge->SetClientKey(clientKey);
  bridge->SetIpAddress("127.0.0.1"); // Can be anything as long as it is a valid address.
  bridge->SetModelId("BSB002"); // We need that otherwise validation will fail.
  bridge->SetIsValidIp(true);
  bridge->SetIsAuthorized(true);

  StartLoading([this, bridge]() {
    _persistentData->SetActiveBridge(bridge);
    StartRetrieveSmallConfig(bridge);
    });
}

void ConnectionFlow::ResetBridge() {
    DISPATCH_P1(DoReset, true);
}

void ConnectionFlow::ResetAll() {
    DISPATCH_P1(DoReset, false);
}

void ConnectionFlow::DoReset(bool onlyActiveBridge) {
    if (onlyActiveBridge && !Start(FeedbackMessage::REQUEST_TYPE_RESET_BRIDGE))
        return;

    if (!onlyActiveBridge && !Start(FeedbackMessage::REQUEST_TYPE_RESET_ALL))
        return;

    if (_persistentData->GetActiveBridge()->IsStreaming()) {
        _stream->Stop(_persistentData->GetActiveBridge());
    }

    ClearBridge();
}

void ConnectionFlow::SelectGroup(std::string id) {
    DISPATCH_P1(DoSelectGroup, id);
}

void ConnectionFlow::DoSelectGroup(std::string id) {
    if (!Start(FeedbackMessage::REQUEST_TYPE_SELECT))
        return;

    auto selected = _persistentData->GetActiveBridge()->SelectGroup(id);
    if (!_persistentData->GetActiveBridge()->IsValidGroupSelected()) {
        ReportActionRequired();
    }

    if (selected) {
        StartSaving();
    } else {
        Finish();
    }
}

void ConnectionFlow::StartStream(StreamPtr stream) {
    if (stream) {
        // replace stream with new one, if provided
        _stream = stream;
    }
    DISPATCH_P0(DoStartStream);
}

void ConnectionFlow::DoStartStream() {
    if (!Start(FeedbackMessage::REQUEST_TYPE_ACTIVATE))
        return;

    StartRetrieveFullConfig();
}

void ConnectionFlow::StopStream(StreamPtr stream) {
    if (stream) {
        // replace stream with new one, if provided
        _stream = stream;
    }
    DISPATCH_P0(DoStopStream);
}

void ConnectionFlow::DoStopStream() {
    if (!Start(FeedbackMessage::REQUEST_TYPE_DEACTIVATE))
        return;

    DeactivateStreaming();
}

void ConnectionFlow::Abort() {
    DISPATCH_P0(DoAbort);
}

void ConnectionFlow::DoAbort() {
    if (_state == Idle)
        return;

    if (_state == SearchingFirst || _state == SearchingRetry) {
        _bridgeSearcher->Abort();
    }

    if (_state == Authenticating) {
        _ongoingAuthenticationCount = 0;
    }

    _persistentData->SetActiveBridge(_bridgeStartState);
    NewMessage(FeedbackMessage(_request, FeedbackMessage::ID_DONE_ABORTED, _persistentData->GetActiveBridge()));
    Finish();
}

void ConnectionFlow::OnBridgeMonitorEvent(const FeedbackMessage & message) {
    DISPATCH_P1(DoOnBridgeMonitorEvent, message);
}

void ConnectionFlow::DoOnBridgeMonitorEvent(const FeedbackMessage & message) {
    if (!Start(FeedbackMessage::REQUEST_TYPE_INTERNAL)) {
        if (message.GetId() == FeedbackMessage::ID_BRIDGE_CONNECTED && message.GetBridge()->IsSupportingClipV2()) {
            NewMessage(message);
        }
        return;
    }

    _persistentData->SetActiveBridge(message.GetBridge());

    if (message.GetId() == FeedbackMessage::ID_STREAMING_DISCONNECTED) {
        _stream->Stop(_persistentData->GetActiveBridge());
    }

    if (message.GetId() == FeedbackMessage::ID_BRIDGE_CONNECTED && message.GetBridge()->IsSupportingClipV2()) {
        // With clipv2 being reconnected means that we need to fully fetch everything again. The config retriever is going to take care of sending the BRIDGE_CONNECTED event.
        StartRetrieveFullConfig();
        return;
    }
    if (message.GetId() == FeedbackMessage::ID_INVALID_EVENTING_CONNECTION) {
        if (_fullConfigRetriever != nullptr) {
            _fullConfigRetriever->RefreshBridgeConnection();
        }
    }
    if (message.GetId() == FeedbackMessage::ID_BRIDGE_REFRESHED) {
        StartSaving();
    }
    else {
        NewMessage(message);
    }

    Finish();
}

bool ConnectionFlow::Start(FeedbackMessage::RequestType request) {
    if (_state != Idle)
        return false;

    _request = request;

    if (request != FeedbackMessage::REQUEST_TYPE_INTERNAL) {
        NewMessage(FeedbackMessage(_request, FeedbackMessage::ID_USERPROCEDURE_STARTED, _persistentData->GetActiveBridge()));
        _bridgeStartState = _persistentData->GetActiveBridge()->Clone();
    }

    return true;
}

void ConnectionFlow::NewMessage(const FeedbackMessage& message) {
    _feedbackMessageCallback(message);
        if (_fullConfigRetriever != nullptr) {
        _fullConfigRetriever->OnBridgeMonitorEvent(message);
    }
}

void ConnectionFlow::StartLoading(std::function<void()> callback) {
    NewMessage(FeedbackMessage(_request, FeedbackMessage::ID_START_LOADING, _persistentData->GetActiveBridge()));
    _state = Loading;
    _storageAccessor->Load([this, callback](OperationResult r, HueStreamDataPtr data) {
        DISPATCH_P3(LoadingCompleted, r, data, callback);
    });
}

void ConnectionFlow::LoadingCompleted(OperationResult result, HueStreamDataPtr persistentData, std::function<void()> callback) {
    if (_state != Loading)
        return;

    _persistentData = persistentData;

    if (result == OPERATION_SUCCESS) {
        NewMessage(FeedbackMessage(_request, FeedbackMessage::ID_FINISH_LOADING_BRIDGE_CONFIGURED, _persistentData->GetActiveBridge(), persistentData->GetAllKnownBridges()));
    } else {
        NewMessage(FeedbackMessage(_request, FeedbackMessage::ID_FINISH_LOADING_NO_BRIDGE_CONFIGURED, _persistentData->GetActiveBridge()));
        _persistentData->GetActiveBridge()->Clear();
    }

    callback();
}


void ConnectionFlow::StartBridgeSearch() {
    NewMessage(FeedbackMessage(_request, FeedbackMessage::ID_START_SEARCHING, _persistentData->GetActiveBridge()));

    bool bruteForce = false;
    if (_state == SearchingFirst) {
        _state = SearchingRetry;
        bruteForce = true;
    } else {
        _state = SearchingFirst;
    }

    _bridgeSearcher = _factory->CreateSearcher();
    _bridgeSearcher->SearchNew(bruteForce, [this](BridgeListPtr bridges) {
        DISPATCH_P1(BridgeSearchCompleted, bridges);
    });
}

void ConnectionFlow::BridgeSearchCompleted(BridgeListPtr bridges) {
    if (_state != SearchingFirst && _state != SearchingRetry)
        return;

    _bridgeSearcher.reset();

    bridges = FilterInvalidAndDuplicateIpAddresses(bridges);

    if (bridges->size() == 0) {
        NewMessage(FeedbackMessage(_request, FeedbackMessage::ID_FINISH_SEARCHING_NO_BRIDGES_FOUND, _persistentData->GetActiveBridge()));

        if (_request == FeedbackMessage::REQUEST_TYPE_CONNECT_BACKGROUND) {
            NewMessage(FeedbackMessage(_request, FeedbackMessage::ID_DONE_NO_BRIDGE_FOUND, _persistentData->GetActiveBridge()));
            Finish();
        } else if (_state == SearchingFirst) {
            StartBridgeSearch();
        } else {
            ReportActionRequired();
            StartSaving();
        }

        return;
    }

    if (ContainsValidBridge(bridges)) {
        NewMessage(FeedbackMessage(_request, FeedbackMessage::ID_FINISH_SEARCH_BRIDGES_FOUND, _persistentData->GetActiveBridge(), bridges));

        if (_request != FeedbackMessage::REQUEST_TYPE_CONNECT_NEW && _persistentData->RediscoverKnownBridge(bridges)) {
            StartRetrieveSmallConfig(_persistentData->GetActiveBridge());
            return;
        }
        if (_request == FeedbackMessage::REQUEST_TYPE_CONNECT_BACKGROUND) {
            NewMessage(FeedbackMessage(_request, FeedbackMessage::ID_DONE_BRIDGE_FOUND, _persistentData->GetActiveBridge(), bridges));
            _backgroundDiscoveredBridges = bridges;
            Finish();
            return;
        }
    } else {
        NewMessage(FeedbackMessage(_request, FeedbackMessage::ID_FINISH_SEARCHING_INVALID_BRIDGES_FOUND, _persistentData->GetActiveBridge()));

        if (_request == FeedbackMessage::REQUEST_TYPE_CONNECT_BACKGROUND) {
            if (_persistentData->GetActiveBridge()->IsEmpty()) {
                NewMessage(FeedbackMessage(_request, FeedbackMessage::ID_DONE_NO_BRIDGE_FOUND, _persistentData->GetActiveBridge()));
                Finish();
            } else {
                ReportActionRequired();
                StartSaving();
            }
            return;
        }
    }

    StartAuthentication(bridges);
}

BridgeListPtr ConnectionFlow::FilterInvalidAndDuplicateIpAddresses(BridgeListPtr bridges) {
    auto uniqueValidBridges = std::make_shared<BridgeList>();
    for (auto x = bridges->begin(); x != bridges->end(); ++x) {
        if ((*x)->IsValidIp() &&
            std::find_if(uniqueValidBridges->begin(), uniqueValidBridges->end(),
                [x](const BridgePtr &arg) {return arg->GetIpAddress() == (*x)->GetIpAddress(); }) == uniqueValidBridges->end()) {
            uniqueValidBridges->push_back(*x);
        }
    }
    return uniqueValidBridges;
}

bool ConnectionFlow::ContainsValidBridge(BridgeListPtr bridges) {
    for (auto bridge : *bridges) {
        if (bridge->IsValidModelId()) {
            return true;
        }
    }
    return false;
}

void ConnectionFlow::StartAuthentication(BridgePtr bridge) {
    auto bridges = std::make_shared<BridgeList>();
    bridges->push_back(bridge);
    StartAuthentication(bridges);
}

void ConnectionFlow::StartAuthentication(BridgeListPtr bridges) {
    NewMessage(FeedbackMessage(_request, FeedbackMessage::ID_START_AUTHORIZING, _persistentData->GetActiveBridge(), bridges));
    NewMessage(FeedbackMessage(_request, FeedbackMessage::ID_PRESS_PUSH_LINK, _persistentData->GetActiveBridge(), bridges));
    _state = Authenticating;
    _ongoingAuthenticationCount = bridges->size();
    _authenticationProcessesInfo.clear();
    _scheduler->start();
    auto i = 0;
    for (auto&& bridge : *bridges) {
        _authenticationProcessesInfo.emplace(bridge, AuthenticationProcessInfo{i++});

        PushLinkBridge(bridge);
    }
}

void ConnectionFlow::AuthenticationCompleted(BridgePtr bridge) {
    if (_state != Authenticating)
        return;

    auto& authenticationProcessInfo = _authenticationProcessesInfo[bridge];

    if (!bridge->IsValidIp()) {
        if (++authenticationProcessInfo.failedTries > PUSHLINK_INVALID_IP_MAX_RETRIES) {
            _ongoingAuthenticationCount--;
            if (_ongoingAuthenticationCount == 0) {
                NewMessage(FeedbackMessage(_request, FeedbackMessage::ID_FINISH_AUTHORIZING_FAILED, _persistentData->GetActiveBridge()));
                StartBridgeSearch();
                _scheduler->remove_all_tasks();
                _scheduler->stop();
            }

            return;
        } else {
            bridge->SetIsValidIp(true);
        }
    }

    if (bridge->IsAuthorized()) {
        _ongoingAuthenticationCount--;
        _persistentData->SetActiveBridge(bridge);
        NewMessage(FeedbackMessage(_request, FeedbackMessage::ID_FINISH_AUTHORIZING_AUTHORIZED, _persistentData->GetActiveBridge(), _persistentData->GetAllKnownBridges()));
        _scheduler->remove_all_tasks();
        _scheduler->stop();
        StartRetrieveFullConfig();
    } else {
        support::SchedulerTask scheduler_task;
        scheduler_task.set_id(authenticationProcessInfo.bridgeNumber);
        scheduler_task.set_interval_ms(PUSHLINK_RETRY_INTERVAL_MS);
        scheduler_task.set_recurring(false);

        // Set the method which need to be executed by the scheduler
        scheduler_task.set_method(std::bind(&ConnectionFlow::SchedulerPushlinkTimedOut, this, bridge));

        // Add task to the scheduler, if task already exists with the same id it will be updated automatically
        _scheduler->add_task(scheduler_task);
    }
}
void ConnectionFlow::SchedulerPushlinkTimedOut(BridgePtr bridge) {
    DISPATCH_P1(PushLinkBridge, bridge);
}

void ConnectionFlow::PushLinkBridge(BridgePtr bridge) {
    _factory->CreateAuthenticator()->Authenticate(bridge, _appSettings, [this](BridgePtr b) {
        DISPATCH_P1(AuthenticationCompleted, b);
    });
}

void ConnectionFlow::StartRetrieveSmallConfig(BridgePtr bridge) {
    _state = Retrieving;

    NewMessage(FeedbackMessage(_request, FeedbackMessage::ID_START_RETRIEVING_SMALL, _persistentData->GetActiveBridge()));

        std::weak_ptr<ConnectionFlow> lifetime = shared_from_this();

    _smallConfigRetriever = _factory->CreateConfigRetriever(_appSettings->UseForcedActivation(), ConfigType::Small);
    _smallConfigRetriever->Execute(bridge, [lifetime, this, bridge](OperationResult result, BridgePtr configured_bridge) {
        std::shared_ptr<ConnectionFlow> ref = lifetime.lock();
        if (ref == nullptr) {
            return;
        }

        bridge->SetId(configured_bridge->GetId());
        bridge->SetModelId(configured_bridge->GetModelId());
        bridge->SetApiversion(configured_bridge->GetApiversion());
        bridge->SetSwversion(configured_bridge->GetSwversion());
        DISPATCH_P2(RetrieveSmallConfigCompleted, result, bridge);
        }, [](const huestream::FeedbackMessage& msg)
        {
        });
}

void ConnectionFlow::RetrieveSmallConfigCompleted(OperationResult result, BridgePtr bridge) {
    if (_state != Retrieving)
        return;

    if (result == OPERATION_FAILED) {
        RetrieveFailed(bridge);
    } else {
        NewMessage(FeedbackMessage(_request, FeedbackMessage::ID_FINISH_RETRIEVING_SMALL, bridge));

        _persistentData->SetActiveBridge(bridge);

        if (!bridge->IsValidModelId()) {
            StartBridgeSearch();
            return;
        }

        if (bridge->IsAuthorized() || bridge->HasEverBeenAuthorizedForStreaming()) {
            StartRetrieveFullConfig();
        } else {
            StartAuthentication(bridge);
        }
    }
}

void ConnectionFlow::StartRetrieveFullConfig() {
    _state = Retrieving;

        BridgePtr bridge = _persistentData->GetActiveBridge();

        std::weak_ptr<ConnectionFlow> lifetime = shared_from_this();

        if (_fullConfigRetriever == nullptr || (bridge->IsSupportingClipV2() && !_fullConfigRetriever->IsSupportingClipV2()) || (!bridge->IsSupportingClipV2() && _fullConfigRetriever->IsSupportingClipV2()))
        {
            // Only recreate a full config retriever if necessary
            _fullConfigRetriever = _factory->CreateConfigRetriever(_appSettings->UseForcedActivation(), ConfigType::Full, bridge->IsSupportingClipV2());
        }

        if (_fullConfigRetriever->Execute(_persistentData->GetActiveBridge(), [lifetime, this](OperationResult result, BridgePtr bridge)
        {
            std::shared_ptr<ConnectionFlow> ref = lifetime.lock();
            if (ref == nullptr)
            {
                return;
            }

            DISPATCH_P2(RetrieveFullConfigCompleted, result, bridge);
        }, [this, lifetime](const huestream::FeedbackMessage& msg)
        {
            std::shared_ptr<ConnectionFlow> ref = lifetime.lock();
            if (ref == nullptr)
            {
                return;
            }

            OnBridgeMonitorEvent(msg);
        }))
        {
            NewMessage(FeedbackMessage(_request, FeedbackMessage::ID_START_RETRIEVING, bridge));
        }
}

void ConnectionFlow::RetrieveFullConfigCompleted(OperationResult result, BridgePtr bridge) {
    if (_state != Retrieving)
        return;

    _persistentData->SetActiveBridge(bridge);

    if (result == OPERATION_FAILED) {
        RetrieveFailed(bridge);
    } else {
        auto status = _persistentData->GetActiveBridge()->GetStatus();
        if (status == BRIDGE_INVALID_MODEL ||
            status == BRIDGE_INVALID_VERSION ||
            status == BRIDGE_INVALID_CLIENTKEY ||
            status == BRIDGE_NO_GROUP_AVAILABLE) {
            NewMessage(FeedbackMessage(_request, FeedbackMessage::ID_FINISH_RETRIEVING_ACTION_REQUIRED, _persistentData->GetActiveBridge()));
            ReportActionRequired();
        } else if ((_persistentData->GetActiveBridge()->IsValidGroupSelected() == false) &&
            (_persistentData->GetActiveBridge()->SelectGroupIfOnlyOneOption() == false)) {
            NewMessage(FeedbackMessage(_request, FeedbackMessage::ID_FINISH_RETRIEVING_ACTION_REQUIRED, _persistentData->GetActiveBridge()));
            ReportActionRequired();
        } else {
            NewMessage(FeedbackMessage(_request, FeedbackMessage::ID_FINISH_RETRIEVING_READY_TO_START, _persistentData->GetActiveBridge()));
        }

        StartSaving();
    }
}

void ConnectionFlow::RetrieveFailed(BridgePtr bridge) {
    NewMessage(FeedbackMessage(_request, FeedbackMessage::ID_FINISH_RETRIEVING_FAILED, bridge));
    if (!bridge->IsValidIp() && bridge->IsAuthorized()) {
        StartBridgeSearch();
        return;
    }
    if (bridge->IsValidIp() && !bridge->IsAuthorized()) {
        if (_request == FeedbackMessage::REQUEST_TYPE_CONNECT_BACKGROUND) {
            NewMessage(FeedbackMessage(_request, FeedbackMessage::ID_DONE_BRIDGE_FOUND, _persistentData->GetActiveBridge()));
            Finish();
        } else {
            StartAuthentication(bridge);
        }
        return;
    }
    bridge->SetIsValidIp(false);
    bridge->SetIsAuthorized(false);
    ReportActionRequired();

    StartSaving();
}

void ConnectionFlow::ReportActionRequired() {
    NewMessage(FeedbackMessage(_request, FeedbackMessage::ID_DONE_ACTION_REQUIRED, _persistentData->GetActiveBridge()));

    BridgeStatus status = _persistentData->GetActiveBridge()->GetStatus();

    switch (status) {
        case BRIDGE_EMPTY:
        case BRIDGE_NOT_FOUND:
            NewMessage(FeedbackMessage(_request, FeedbackMessage::ID_NO_BRIDGE_FOUND, _persistentData->GetActiveBridge()));
            break;
        case BRIDGE_INVALID_MODEL:
            NewMessage(FeedbackMessage(_request, FeedbackMessage::ID_INVALID_MODEL, _persistentData->GetActiveBridge()));
            break;
        case BRIDGE_NOT_CONNECTABLE:
        case BRIDGE_NOT_CONNECTED:
            NewMessage(FeedbackMessage(_request, FeedbackMessage::ID_BRIDGE_NOT_FOUND, _persistentData->GetActiveBridge()));
            break;
        case BRIDGE_INVALID_VERSION:
            NewMessage(FeedbackMessage(_request, FeedbackMessage::ID_INVALID_VERSION, _persistentData->GetActiveBridge()));
            break;
        case BRIDGE_NO_GROUP_AVAILABLE:
            NewMessage(FeedbackMessage(_request, FeedbackMessage::ID_NO_GROUP_AVAILABLE, _persistentData->GetActiveBridge()));
            break;
        case BRIDGE_INVALID_GROUP_SELECTED:
            NewMessage(FeedbackMessage(_request, FeedbackMessage::ID_SELECT_GROUP, _persistentData->GetActiveBridge()));
            break;
        case BRIDGE_BUSY:
            NewMessage(FeedbackMessage(_request, FeedbackMessage::ID_BUSY_STREAMING, _persistentData->GetActiveBridge()));
            break;
        default:
            NewMessage(FeedbackMessage(_request, FeedbackMessage::ID_INTERNAL_ERROR, _persistentData->GetActiveBridge()));
            break;
    }
}

void ConnectionFlow::ClearBridge() {
    if (_request == FeedbackMessage::REQUEST_TYPE_RESET_BRIDGE) {
        _persistentData->ClearActiveBridge();
    }
    if (_request == FeedbackMessage::REQUEST_TYPE_RESET_ALL) {
        _persistentData->Clear();
    }

    NewMessage(FeedbackMessage(_request, FeedbackMessage::ID_DONE_RESET, _persistentData->GetActiveBridge()));

    StartSaving();
}

void ConnectionFlow::StartSaving() {
    NewMessage(FeedbackMessage(_request, FeedbackMessage::ID_START_SAVING, _persistentData->GetActiveBridge(), _persistentData->GetAllKnownBridges()));
    _state = Saving;

    _storageAccessor->Save(_persistentData, [this](OperationResult r) {
        DISPATCH_P1(SavingCompleted, r);
    });
}

void ConnectionFlow::SavingCompleted(OperationResult r) {
    if (r == OPERATION_SUCCESS) {
        NewMessage(FeedbackMessage(_request, FeedbackMessage::ID_FINISH_SAVING_SAVED, _persistentData->GetActiveBridge(), _persistentData->GetAllKnownBridges()));
    } else {
        NewMessage(FeedbackMessage(_request, FeedbackMessage::ID_FINISH_SAVING_FAILED, _persistentData->GetActiveBridge()));
    }

    _persistentData->GetActiveBridge()->SetIsBusy(false);
    auto switchGroupWhileStreaming = (_bridgeStartState->IsStreaming() && _request == FeedbackMessage::REQUEST_TYPE_SELECT);
    auto activationRequired = (_request == FeedbackMessage::REQUEST_TYPE_ACTIVATE || _appSettings->AutoStartAtConnection() || switchGroupWhileStreaming);
    auto canActivateStreaming = (_persistentData->GetActiveBridge()->IsReadyToStream() || _persistentData->GetActiveBridge()->IsStreaming());

    if (activationRequired && canActivateStreaming) {
        ActivateStreaming();
    } else if (!activationRequired && _persistentData->GetActiveBridge()->IsReadyToStream()) {
        // If we do not have to start the connection, then we are done ...
        NewMessage(FeedbackMessage(_request, FeedbackMessage::ID_DONE_COMPLETED, _persistentData->GetActiveBridge()));
        Finish();
        return;
    } else {
        // Assume that ReportActionRequired() has been executed
        Finish();
        return;
    }
}

void ConnectionFlow::ActivateStreaming() {
    NewMessage(FeedbackMessage(_request, FeedbackMessage::ID_START_ACTIVATING, _persistentData->GetActiveBridge()));
    _state = Activating;

    bool result;
    if (_appSettings->UseRenderThread()) {
        result = _stream->StartWithRenderThread(_persistentData->GetActiveBridge());
    } else {
        result = _stream->Start(_persistentData->GetActiveBridge());
    }

    if (result) {
        NewMessage(FeedbackMessage(_request, FeedbackMessage::ID_FINISH_ACTIVATING_ACTIVE, _persistentData->GetActiveBridge()));
        NewMessage(FeedbackMessage(_request, FeedbackMessage::ID_DONE_COMPLETED, _persistentData->GetActiveBridge()));
    } else {
        NewMessage(FeedbackMessage(_request, FeedbackMessage::ID_FINISH_ACTIVATING_FAILED, _persistentData->GetActiveBridge()));
        ReportActionRequired();
    }

    Finish();
}

void ConnectionFlow::DeactivateStreaming() {
    _state = Deactivating;

    _stream->Stop(_persistentData->GetActiveBridge());

    Finish();
}

void ConnectionFlow::Finish() {
    _state = Idle;

    if (_request != FeedbackMessage::REQUEST_TYPE_INTERNAL) {
        auto differences = CompareBridges(_bridgeStartState, _persistentData->GetActiveBridge());
        for (auto const& feedbackId : differences) {
            NewMessage(FeedbackMessage(_request, feedbackId, _persistentData->GetActiveBridge()));
        }
        NewMessage(FeedbackMessage(_request, FeedbackMessage::ID_USERPROCEDURE_FINISHED, _persistentData->GetActiveBridge()));
    }

    _request = FeedbackMessage::REQUEST_TYPE_INTERNAL;
}
ConnectionFlowState ConnectionFlow::GetState() {
    return _state;
}

void ConnectionFlow::SetFeedbackMessageCallback(FeedbackMessageCallback callback) {
    _feedbackMessageCallback = callback;
}

}  // namespace huestream
