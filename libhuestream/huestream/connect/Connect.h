/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/
/** @file */

#ifndef HUESTREAM_CONNECT_CONNECT_H_
#define HUESTREAM_CONNECT_CONNECT_H_

#include "huestream/connect/ConnectionFlow.h"
#include "huestream/config/Config.h"
#include "huestream/connect/IMessageDispatcher.h"
#include "huestream/connect/IConnectionFlowFactory.h"
#include "huestream/connect/ConnectionFlowFactory.h"
#include "huestream/common/storage/IStorageAccessor.h"
#include "huestream/connect/IFeedbackMessageHandler.h"

#include <memory>
#include <string>

namespace huestream {


    /**
     result of a connection procedure
     */
    enum ConnectResult {
        Uninitialized,   ///< connection procedure has not been started
        Busy,            ///< connection procedure is ongoing
        NoBridgeFound,   ///< connection procedure finished without finding a bridge
        BridgeFound,     ///< connection procedure discovered a bridge and is awaiting call to connect to it
        ActionRequired,  ///< connection procedure was interrupted because a user action is required to be able to start streaming
        ReadyToStart,    ///< connection procedure connected to a bridge and is awaiting call to start streaming
        Streaming        ///< connection procedure completed with an active streaming session
    };

    class Connect;
    class Config;

    typedef std::shared_ptr<Config> ConfigPtr;

    typedef std::function<void()> DoneCallback;

    class IConnect {
     public:
        virtual ~IConnect() = default;

        virtual void Load() = 0;

        virtual void ConnectToBridge() = 0;

        virtual void ConnectToBridgeBackground() = 0;

        virtual void ConnectToBridgeWithIp(const std::string &ipAddress) = 0;

        virtual void ConnectToNewBridge() = 0;

        virtual void SetManual(BridgePtr bridge) = 0;

        virtual void ConnectoToBridgeWithIdKey(const std::string& id, const std::string& user, const std::string& clientKey) = 0;

        virtual void ResetBridge() = 0;

        virtual void ResetAllData() = 0;

        virtual void SelectGroup(std::string id) = 0;

        virtual void StartStream(StreamPtr stream) = 0;

        virtual void StopStream(StreamPtr stream) = 0;

        virtual void Abort() = 0;

        virtual void OnBridgeMonitorEvent(const FeedbackMessage &message) = 0;

        virtual void NewMessage(const FeedbackMessage &message) = 0;

        virtual void SetFeedbackMessageCallback(FeedbackMessageCallback callback) = 0;

        virtual void WaitUntilReady() = 0;

        virtual bool IsReady() = 0;

        virtual ConnectResult GetResult() = 0;
    };

    typedef std::shared_ptr<IConnect> ConnectPtr;

    class Connect : public IConnect {
    public:
        Connect(BridgeHttpClientPtr http, MessageDispatcherPtr dispatcher, BridgeSettingsPtr bridgeSettings, AppSettingsPtr appSettings, StreamPtr stream, BridgeStorageAccessorPtr storageAccessor);

        void SetFeedbackMessageCallback(FeedbackMessageCallback callback) override;

        void SetFeedbackHandler(FeedbackMessageHandlerPtr handler);

        void WaitUntilReady() override;

        bool IsReady() override;

        ConnectResult GetResult() override;

        void Load() override;

        void ConnectToBridge() override;

        void ConnectToBridgeBackground() override;

        void ConnectToBridgeWithIp(const std::string &ipAddress) override;

        void ConnectToNewBridge() override;

        void SetManual(BridgePtr bridge) override;

        void ConnectoToBridgeWithIdKey(const std::string& id, const std::string& user, const std::string& clientKey) override;

        void ResetBridge() override;

        void ResetAllData() override;

        void SelectGroup(std::string id) override;

        void StartStream(StreamPtr stream) override;

        void StopStream(StreamPtr stream) override;

        void Abort() override;

        void OnBridgeMonitorEvent(const FeedbackMessage &message) override;

        void NewMessage(const FeedbackMessage &message) override;

    protected:
        std::mutex _mutex;
        std::condition_variable _condition;
        bool _isIdle;
        ConnectResult _result;
        FeedbackMessageCallback _messageCallback;
        FeedbackMessageHandlerPtr _feedbackHandler;
        ConnectionFlowPtr _connectionFlow;

        void UpdateConnectionStatus(const FeedbackMessage &message);
        void TranslateBridgeStatusToConnectionResult(const BridgeStatus status);
        void ReportFeedbackMessage(const FeedbackMessage &message);
    };

}  // namespace huestream

#endif  // HUESTREAM_CONNECT_CONNECT_H_
