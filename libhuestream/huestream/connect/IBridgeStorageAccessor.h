/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#ifndef HUESTREAM_CONNECT_IBRIDGESTORAGEACCESSOR_H_
#define HUESTREAM_CONNECT_IBRIDGESTORAGEACCESSOR_H_

#include "huestream/connect/OperationResult.h"
#include "huestream/common/data/HueStreamData.h"

#include <memory>
#include <functional>

namespace huestream {

    typedef std::function<void(OperationResult, HueStreamDataPtr)> BridgesLoadCallbackHandler;
    typedef std::function<void(OperationResult)> BridgesSaveCallbackHandler;


    class IBridgeStorageAccessor {
    public:
        virtual ~IBridgeStorageAccessor() {}

        virtual void Load(BridgesLoadCallbackHandler cb) = 0;

        virtual void Save(HueStreamDataPtr bridges, BridgesSaveCallbackHandler cb) = 0;
    };

    typedef std::shared_ptr<IBridgeStorageAccessor> BridgeStorageAccessorPtr;

}  // namespace huestream

#endif  // HUESTREAM_CONNECT_IBRIDGESTORAGEACCESSOR_H_
