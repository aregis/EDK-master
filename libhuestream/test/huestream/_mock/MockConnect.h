/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#ifndef LIBHUESTREAM_MOCKHUECONNECT_H
#define LIBHUESTREAM_MOCKHUECONNECT_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <memory>

#include "huestream/connect/IConnectionFlow.h"

namespace huestream {

    class MockConnect : public IConnect {
    public:
        FeedbackMessageCallback callback;
        MOCK_METHOD0(Load, void());
        MOCK_METHOD0(ConnectToBridge, void());
        MOCK_METHOD0(ConnectToBridgeBackground, void());
        MOCK_METHOD1(ConnectToBridgeWithIp, void(const std::string &ipAddress));
        MOCK_METHOD0(ConnectToNewBridge, void());
        MOCK_METHOD1(SetManual, void(BridgePtr bridge));
        MOCK_METHOD3(ConnectoToBridgeWithIdKey, void(const std::string& id, const std::string& user, const std::string& clientKey));
        MOCK_METHOD0(ResetBridge, void());
        MOCK_METHOD0(ResetAllData, void());
        MOCK_METHOD1(SelectGroup, void(std::string id));
        MOCK_METHOD1(StartStream, void(StreamPtr stream));
        MOCK_METHOD1(StopStream, void(StreamPtr stream));
        MOCK_METHOD0(Abort, void());
        MOCK_METHOD1(OnBridgeMonitorEvent, void(const huestream::FeedbackMessage &message));
        MOCK_METHOD1(NewMessage, void(const huestream::FeedbackMessage &message));

        MOCK_METHOD0(WaitUntilReady, void());
        MOCK_METHOD0(IsReady, bool());
        MOCK_METHOD0(GetResult, ConnectResult());
        MOCK_METHOD1(SetFeedbackMessageCallback, void(FeedbackMessageCallback callback));
    };

    class MockWrapperConnect : public IConnect {
    public:
        explicit MockWrapperConnect(const std::shared_ptr<IConnect>& mock)
           : _mock(mock) {}

        void Load() {
            _mock->Load();
        }

        void ConnectToBridge() {
            _mock->ConnectToBridge();
        }

        void ConnectToBridgeBackground() {
            _mock->ConnectToBridgeBackground();
        }

        void ConnectToBridgeWithIp(const std::string& ipAddress) {
            _mock->ConnectToBridgeWithIp(ipAddress);
        }

        void ConnectToNewBridge() {
            _mock->ConnectToNewBridge();
        }

        void SetManual(BridgePtr bridge) {
            _mock->SetManual(bridge);
        }

        void ConnectoToBridgeWithIdKey(const std::string& id, const std::string& user, const std::string& clientKey) {
          _mock->ConnectoToBridgeWithIdKey(id, user, clientKey);
        }

        void ResetBridge() {
            _mock->ResetBridge();
        }

        void ResetAllData() {
            _mock->ResetAllData();
        }

        void SelectGroup(std::string id) {
            _mock->SelectGroup(id);
        }

        void StartStream(StreamPtr stream) {
            _mock->StartStream(stream);
        }

        void StopStream(StreamPtr stream) {
            _mock->StopStream(stream);
        }

        void Abort() {
            _mock->Abort();
        }

        void OnBridgeMonitorEvent(const huestream::FeedbackMessage& message) {
            _mock->OnBridgeMonitorEvent(message);
        }

        void NewMessage(const huestream::FeedbackMessage& message) {
            _mock->NewMessage(message);
        }

        void WaitUntilReady() {
            _mock->WaitUntilReady();
        }

        bool IsReady() {
            return _mock->IsReady();
        }

        ConnectResult GetResult() {
            return _mock->GetResult();
        }

        void SetFeedbackMessageCallback(FeedbackMessageCallback cb) {
            _mock->SetFeedbackMessageCallback(cb);
        }

    private:
        std::shared_ptr<IConnect> _mock;
    };

}  // namespace huestream
#endif  // LIBHUESTREAM_MOCKHUECONNECT_H
