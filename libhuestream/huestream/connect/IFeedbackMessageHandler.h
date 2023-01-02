/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#ifndef HUESTREAM_CONNECT_IFEEDBACKMESSAGEHANDLER_H_
#define HUESTREAM_CONNECT_IFEEDBACKMESSAGEHANDLER_H_

#include "huestream/connect/ConnectionFlow.h"

#include <memory>

namespace huestream {

    class IFeedbackMessageHandler {
    public:
        virtual ~IFeedbackMessageHandler() = default;

        virtual void NewFeedbackMessage(const FeedbackMessage &message) const = 0;
    };

    typedef std::shared_ptr<IFeedbackMessageHandler> FeedbackMessageHandlerPtr;

}  // namespace huestream

#endif  // HUESTREAM_CONNECT_IFEEDBACKMESSAGEHANDLER_H_
