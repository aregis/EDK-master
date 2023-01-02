/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#ifndef HUESTREAM_CONNECT_IBRIDGESEARCHER_H_
#define HUESTREAM_CONNECT_IBRIDGESEARCHER_H_

#include "huestream/common/data/Bridge.h"

#include <memory>

using huestream::BridgeListPtr;

namespace huestream {

typedef std::function<void(BridgeListPtr)> SearchCallbackHandler;

class IBridgeSearcher {
 public:
    virtual ~IBridgeSearcher() {}

    virtual void SearchNew(SearchCallbackHandler cb) = 0;

    virtual void SearchNew(bool bruteForce, SearchCallbackHandler cb) = 0;

    virtual void Abort() = 0;

 protected:
};

typedef std::shared_ptr<IBridgeSearcher> BridgeSearcherPtr;
}  // namespace huestream

#endif  // HUESTREAM_CONNECT_IBRIDGESEARCHER_H_
