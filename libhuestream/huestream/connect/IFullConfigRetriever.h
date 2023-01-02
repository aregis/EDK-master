/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#ifndef HUESTREAM_CONNECT_IFULLCONFIGRETRIEVER_H_
#define HUESTREAM_CONNECT_IFULLCONFIGRETRIEVER_H_

#include "huestream/common/data/Bridge.h"
#include "huestream/connect/OperationResult.h"
#include "huestream/connect/FeedbackMessage.h"

#include <functional>
#include <memory>

namespace huestream {
    enum class ConfigType {
        Small,
        Full
    };

    typedef std::function<void(OperationResult, BridgePtr)> RetrieveCallbackHandler;
    typedef std::function<void(const huestream::FeedbackMessage &)> FeedbackHandler;

    class IConfigRetriever {
    public:
        virtual ~IConfigRetriever() = default;

        virtual bool Execute(BridgePtr bridge, RetrieveCallbackHandler cb, FeedbackHandler fh) = 0;
        virtual void OnBridgeMonitorEvent(const FeedbackMessage& message) = 0;
        virtual bool IsSupportingClipV2() = 0;
        virtual void RefreshBridgeConnection() = 0;
    };

    using ConfigRetrieverPtr = std::shared_ptr<IConfigRetriever>;
    using FullConfigRetrieverPtr = ConfigRetrieverPtr;
    using IFullConfigRetriever = IConfigRetriever;
}  // namespace huestream

#endif  // HUESTREAM_CONNECT_IFULLCONFIGRETRIEVER_H_
