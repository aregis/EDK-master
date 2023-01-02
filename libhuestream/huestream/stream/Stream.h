/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#ifndef HUESTREAM_STREAM_STREAM_H_
#define HUESTREAM_STREAM_STREAM_H_

#include "huestream/common/data/Bridge.h"
#include "huestream/common/time/ITimeManager.h"
#include "huestream/config/AppSettings.h"
#include "huestream/stream/ProtocolSerializer.h"
#include "huestream/stream/IStream.h"
#include "huestream/stream/IStreamStarter.h"
#include "huestream/stream/IStreamFactory.h"
#include "huestream/stream/StreamSettings.h"

#include <thread>
#include <memory>
#include <atomic>
#include <mutex>

namespace huestream {

    class Stream : public IStream {
    public:
        Stream(StreamSettingsPtr streamSettings, AppSettingsPtr appSettings, TimeManagerPtr timeManager, ConnectorPtr connector);

        Stream(StreamSettingsPtr streamSettings, AppSettingsPtr appSettings, TimeManagerPtr timeManager, ConnectorPtr connector, std::shared_ptr<IStreamFactory> factory);

        ~Stream() override;

        void SetRenderCallback(StreamRenderCallback callback) override;

        bool Start(BridgePtr bridge) override;

        bool StartWithRenderThread(BridgePtr bridge) override;

        void Stop() override;

        void Stop(BridgePtr bridge) override;

        bool IsStreaming() const override;

        void RenderSingleFrame() override;

        int32_t GetStreamCounter() const override;
        void UpdateBridgeGroup(BridgePtr bridge) override;

     protected:
        virtual void RenderThread();

        int64_t UpdateAndGetCurrentTime() const;

        bool IsStreamingToBridgeAndGroup(BridgePtr bridge) const;

        StreamSettingsPtr _streamSettings;
        AppSettingsPtr _appSettings;
        BridgePtr _activeBridgeCopy;
        StreamRenderCallback _renderCallback;
        std::shared_ptr<IStreamFactory> _factory;
        std::shared_ptr<std::thread> _renderThread;
        std::atomic<bool> _running;
        std::atomic<uint8_t> _seqNr;
        std::shared_ptr<ITimeManager> _timeManager;
        std::shared_ptr<StreamOptions> _options;
        ConnectorPtr _connector;
        std::mutex _lock;

        std::atomic<int32_t> _streamCounter;
        bool IsSameBridgeAndGroup(BridgePtr bridge) const;
        bool StartStreamingSession(BridgePtr bridge);
    };

}  // namespace huestream

#endif  // HUESTREAM_STREAM_STREAM_H_
