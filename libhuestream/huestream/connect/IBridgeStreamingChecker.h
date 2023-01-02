/*******************************************************************************
Copyright (C) 2019 Signify Holding
All Rights Reserved.
********************************************************************************/

#ifndef HUESTREAM_CONNECT_IBRIDGESTREAMINGCHECKER_H_
#define HUESTREAM_CONNECT_IBRIDGESTREAMINGCHECKER_H_

#include "huestream/common/data/Bridge.h"
#include "huestream/connect/FeedbackMessage.h"
#include <memory>

namespace huestream {

class IBridgeStreamingChecker {
public:
    virtual ~IBridgeStreamingChecker() = default;

    virtual void Check(BridgePtr bridge) = 0;
    virtual void SetFeedbackMessageCallback(std::function<void(const huestream::FeedbackMessage &)> callback) = 0;
};

using BridgeStateCheckerPtr = std::shared_ptr<IBridgeStreamingChecker>;
}  // namespace huestream

#endif  // HUESTREAM_CONNECT_IBRIDGESTREAMINGCHECKER_H_
