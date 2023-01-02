/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#ifndef LIBHUESTREAM_MOCKSTREAM_H
#define LIBHUESTREAM_MOCKSTREAM_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <memory>

#include "huestream/stream/IStream.h"

MATCHER_P(MatchBridgeId, bridge, "Request did not match") { return arg->GetId() == bridge->GetId(); };

namespace huestream {
    class MockStream : public IStream {
        StreamRenderCallback _callback;

    public:
        void ExecuteRenderCallback(){_callback();};
        void SetRenderCallback(StreamRenderCallback callback) { _callback = callback;};

        MOCK_METHOD1(Start, bool(BridgePtr bridge));

        MOCK_METHOD1(StartWithRenderThread, bool(BridgePtr bridge));

        MOCK_METHOD0(Stop, void());

        MOCK_METHOD1(Stop, void(BridgePtr bridge));

        MOCK_CONST_METHOD0(IsStreaming, bool());

        MOCK_METHOD0(RenderSingleFrame, void());

        MOCK_CONST_METHOD0(GetStreamCounter, int32_t());

        MOCK_METHOD1(UpdateBridgeGroup, void(BridgePtr bridge));
    };

    class MockWrapperStream : public IStream {
    public:
        explicit MockWrapperStream(const std::shared_ptr<MockStream>& mock)
          : _mock(mock) {}

        void ExecuteRenderCallback() {
            _mock->ExecuteRenderCallback();
        }

        void SetRenderCallback(StreamRenderCallback callback) {
            _mock->SetRenderCallback(callback);
        }

        bool Start(BridgePtr bridge) {
            return _mock->Start(bridge);
        }

        bool StartWithRenderThread(BridgePtr bridge) {
            return _mock->StartWithRenderThread(bridge);
        }

        void Stop() {
            _mock->Stop();
        }

        void Stop(BridgePtr bridge) {
            _mock->Stop(bridge);
        }

        bool IsStreaming() const {
            return _mock->IsStreaming();
        }

        void RenderSingleFrame() {
            _mock->RenderSingleFrame();
        }

        int32_t GetStreamCounter() const {
            return _mock->GetStreamCounter();
        }

        void UpdateBridgeGroup(BridgePtr bridge) {
            _mock->UpdateBridgeGroup(bridge);
        }

    private:
        std::shared_ptr<MockStream> _mock;
    };
}

#endif  // LIBHUESTREAM_MOCKSTREAM_H
