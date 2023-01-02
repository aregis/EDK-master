/*******************************************************************************
Copyright (C) 2019 Signify Holding
All Rights Reserved.
********************************************************************************/

#ifndef HUESTREAM_CONNECT_BRIDGESTREAMINGCHECKER_H_
#define HUESTREAM_CONNECT_BRIDGESTREAMINGCHECKER_H_

#include "huestream/connect/IBridgeStreamingChecker.h"
#include "huestream/connect/IFullConfigRetriever.h"
#include "huestream/connect/FeedbackMessage.h"
#include "huestream/common/http/BridgeHttpClient.h"
#include "support/network/http/HttpRequestError.h"

namespace huestream {

class BridgeStreamingChecker: public IBridgeStreamingChecker, public std::enable_shared_from_this<BridgeStreamingChecker> {
public:
    explicit BridgeStreamingChecker(FullConfigRetrieverPtr _fullConfigRetrieverPtr, const BridgeHttpClientPtr http);
    void SetFeedbackMessageCallback(std::function<void(const huestream::FeedbackMessage &)> callback) override;
    void Check(BridgePtr bridge) override;

private:
    FullConfigRetrieverPtr _fullConfigRetrieverPtr;
    BridgeHttpClientPtr _http;
    std::unique_ptr<support::HttpRequestExecutor> _executor;

    std::function<void(const huestream::FeedbackMessage &)> _messageCallback;
};

std::vector<FeedbackMessage::Id> CompareBridges(BridgePtr oldBridge, BridgePtr newBridge);

}  // namespace huestream

#endif  // HUESTREAM_CONNECT_BRIDGESTREAMINGCHECKER_H_
