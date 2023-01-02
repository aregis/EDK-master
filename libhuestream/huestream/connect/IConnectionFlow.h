/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#ifndef HUESTREAM_CONNECT_ICONNECTIONFLOW_H_
#define HUESTREAM_CONNECT_ICONNECTIONFLOW_H_

#include "huestream/common/data/Bridge.h"
#include "huestream/stream/IStream.h"
#include "huestream/connect/FeedbackMessage.h"
#include "huestream/connect/IConnectionFlowFactory.h"
#include "huestream/connect/IBridgeSearcher.h"

#include <memory>
#include <string>
#include <functional>
#include <map>

namespace huestream {

typedef std::function<void(const FeedbackMessage &message)> FeedbackMessageCallback;

enum ConnectionFlowState {
    Idle,
    Loading,
    SearchingFirst,
    SearchingRetry,
    Authenticating,
    Retrieving,
    Saving,
    Activating,
    Deactivating
};

class IConnectionFlow {
 public:
    virtual ~IConnectionFlow() = default;

    virtual void Load() = 0;

    virtual void ConnectToBridge() = 0;

    virtual void ConnectToBridgeBackground() = 0;

    virtual void ConnectToBridgeWithIp(const std::string &ipAddress) = 0;

    virtual void ConnectToNewBridge() = 0;

    virtual void SetManual(BridgePtr bridge) = 0;

    virtual void ConnectoToBridgeWithIdKey(const std::string& id, const std::string& user, const std::string& clientKey) = 0;

    virtual void ResetBridge() = 0;

    virtual void ResetAll() = 0;

    virtual void SelectGroup(std::string id) = 0;

    virtual void StartStream(StreamPtr stream) = 0;

    virtual void StopStream(StreamPtr stream) = 0;

    virtual void Abort() = 0;

    virtual void OnBridgeMonitorEvent(const FeedbackMessage &message) = 0;

    virtual ConnectionFlowState GetState() = 0;

    virtual void NewMessage(const FeedbackMessage &message) = 0;

    virtual void SetFeedbackMessageCallback(FeedbackMessageCallback callback) = 0;
};

typedef std::shared_ptr<IConnectionFlow> ConnectionFlowPtr;
}  // namespace huestream


#endif  // HUESTREAM_CONNECT_ICONNECTIONFLOW_H_
