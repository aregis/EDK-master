/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#ifndef HUESTREAM_STREAM_ISTREAM_H_
#define HUESTREAM_STREAM_ISTREAM_H_

#include "huestream/common/data/Bridge.h"
#include "huestream/stream/IConnector.h"

#include <functional>
#include <memory>

namespace huestream {

    typedef std::function<void()> StreamRenderCallback;

    class IStream {
    public:
        virtual ~IStream() = default;

        virtual void SetRenderCallback(StreamRenderCallback callback) = 0;

        virtual bool Start(BridgePtr bridge) = 0;

        virtual bool StartWithRenderThread(BridgePtr bridge) = 0;

        virtual void Stop() = 0;

        virtual void Stop(BridgePtr bridge) = 0;

        virtual bool IsStreaming() const = 0;

        virtual void RenderSingleFrame() = 0;

        virtual int32_t GetStreamCounter() const = 0;

        virtual void UpdateBridgeGroup(BridgePtr bridge) = 0;
    };

    typedef std::shared_ptr<IStream> StreamPtr;
}  // namespace huestream

#endif  // HUESTREAM_STREAM_ISTREAM_H_
