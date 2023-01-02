/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <huestream/common/storage/IStorageAccessor.h>
#include <huestream/connect/IConnectionFlowFactory.h>
#include <huestream/connect/IMessageDispatcher.h>
#include <huestream/connect/Connect.h>

#include <memory>
#include <string>

using std::unique_lock;
using std::mutex;
using std::string;
using huestream::IConnectionFlowFactory;
using huestream::ConnectionFlowFactoryPtr;
using huestream::ConnectionFlowFactory;

namespace huestream {

Connect::Connect(BridgeHttpClientPtr http, MessageDispatcherPtr dispatcher, BridgeSettingsPtr bridgeSettings, AppSettingsPtr appSettings, StreamPtr stream, BridgeStorageAccessorPtr storageAccessor) :
    _isIdle(true),
    _result(Uninitialized),
    _connectionFlow(std::make_shared<ConnectionFlow>(
        std::static_pointer_cast<IConnectionFlowFactory>(std::make_shared<ConnectionFlowFactory>(bridgeSettings, http, dispatcher, storageAccessor)), stream, bridgeSettings, appSettings, storageAccessor)) {
    _connectionFlow->SetFeedbackMessageCallback([this](const FeedbackMessage &message){
      NewMessage(message);
    });
}

void Connect::SetFeedbackMessageCallback(FeedbackMessageCallback callback) {
    _messageCallback = callback;
}

void Connect::SetFeedbackHandler(FeedbackMessageHandlerPtr handler) {
    _feedbackHandler = handler;
}

void Connect::WaitUntilReady() {
    unique_lock<mutex> lock(_mutex);
    _condition.wait(lock, [this]() -> bool { return _isIdle; });
}

bool Connect::IsReady() {
    unique_lock<mutex> lock(_mutex);
    return _isIdle;
}

ConnectResult Connect::GetResult() {
    unique_lock<mutex> lock(_mutex);
    auto result = _result;
    if (!_isIdle) {
        result = Busy;
    }
    return result;
}

void Connect::ConnectToBridge() {
    unique_lock<mutex> lock(_mutex);
    _isIdle = false;
    _connectionFlow->ConnectToBridge();
}

void Connect::ConnectToBridgeBackground() {
    unique_lock<mutex> lock(_mutex);
    _isIdle = false;
    _connectionFlow->ConnectToBridgeBackground();
}

void Connect::ConnectToBridgeWithIp(const string &ipAddress) {
    unique_lock<mutex> lock(_mutex);
    _isIdle = false;
    _connectionFlow->ConnectToBridgeWithIp(ipAddress);
}

void Connect::ConnectToNewBridge() {
    unique_lock<mutex> lock(_mutex);
    _isIdle = false;
    _connectionFlow->ConnectToNewBridge();
}

void Connect::SetManual(BridgePtr bridge) {
    unique_lock<mutex> lock(_mutex);
    _isIdle = false;
    _connectionFlow->SetManual(bridge);
}

void Connect::ConnectoToBridgeWithIdKey(const std::string& id, const std::string& user, const std::string& clientKey) {
  unique_lock<mutex> lock(_mutex);
  _isIdle = false;
  _connectionFlow->ConnectoToBridgeWithIdKey(id, user, clientKey);
}

void Connect::Abort() {
    unique_lock<mutex> lock(_mutex);
    _connectionFlow->Abort();
}

void Connect::ResetBridge() {
    unique_lock<mutex> lock(_mutex);
    _isIdle = false;
    _connectionFlow->ResetBridge();
}

void Connect::ResetAllData() {
    unique_lock<mutex> lock(_mutex);
    _isIdle = false;
    _connectionFlow->ResetAll();
}

void Connect::SelectGroup(std::string id) {
    unique_lock<mutex> lock(_mutex);
    _isIdle = false;
    _connectionFlow->SelectGroup(id);
}

void Connect::Load() {
    unique_lock<mutex> lock(_mutex);
    _isIdle = false;
    _connectionFlow->Load();
}

void Connect::StartStream(StreamPtr stream) {
    unique_lock<mutex> lock(_mutex);
    _connectionFlow->StartStream(stream);
}

void Connect::StopStream(StreamPtr stream) {
    unique_lock<mutex> lock(_mutex);
    _connectionFlow->StopStream(stream);
}

void Connect::OnBridgeMonitorEvent(const FeedbackMessage &message) {
    _connectionFlow->OnBridgeMonitorEvent(message);
}

void Connect::NewMessage(const FeedbackMessage &message) {
    unique_lock<mutex> lock(_mutex);

    UpdateConnectionStatus(message);
    ReportFeedbackMessage(message);

    if (message.GetId() == FeedbackMessage::ID_USERPROCEDURE_FINISHED) {
        _condition.notify_all();
    }
}

void Connect::UpdateConnectionStatus(const FeedbackMessage &message) {
    switch (message.GetId()) {
    case FeedbackMessage::ID_DONE_RESET:
        _result = Uninitialized;
        break;
    case FeedbackMessage::ID_DONE_NO_BRIDGE_FOUND:
        _result = NoBridgeFound;
        break;
    case FeedbackMessage::ID_DONE_BRIDGE_FOUND:
        _result = BridgeFound;
        break;
    case FeedbackMessage::ID_DONE_ACTION_REQUIRED:
    case FeedbackMessage::ID_DONE_COMPLETED:
    case FeedbackMessage::ID_BRIDGE_DISCONNECTED:
    case FeedbackMessage::ID_STREAMING_DISCONNECTED:
        TranslateBridgeStatusToConnectionResult(message.GetBridge()->GetStatus());
        break;
    case FeedbackMessage::ID_USERPROCEDURE_STARTED:
        _isIdle = false;
        break;
    case FeedbackMessage::ID_USERPROCEDURE_FINISHED:
        _isIdle = true;
        break;
      default:
        break;
    }
}

void Connect::TranslateBridgeStatusToConnectionResult(const BridgeStatus status) {
    switch (status) {
    case BRIDGE_EMPTY:
    case BRIDGE_NOT_FOUND:
    case BRIDGE_INVALID_MODEL:
        _result = NoBridgeFound;
        break;
    case BRIDGE_NOT_CONNECTABLE:
    case BRIDGE_NOT_CONNECTED:
    case BRIDGE_INVALID_VERSION:
    case BRIDGE_INVALID_CLIENTKEY:
    case BRIDGE_NO_GROUP_AVAILABLE:
    case BRIDGE_INVALID_GROUP_SELECTED:
    case BRIDGE_BUSY:
        _result = ActionRequired;
        break;
    case BRIDGE_READY:
        _result = ReadyToStart;
        break;
    case BRIDGE_STREAMING:
        _result = Streaming;
        break;
    default:
        break;
    }
}

void Connect::ReportFeedbackMessage(const FeedbackMessage &message) {
    if (_messageCallback) {
        _messageCallback(message);
    }

    if (_feedbackHandler) {
        _feedbackHandler->NewFeedbackMessage(message);
    }
}

}  // namespace huestream
