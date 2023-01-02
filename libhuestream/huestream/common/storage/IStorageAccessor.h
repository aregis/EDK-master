/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#ifndef HUESTREAM_COMMON_STORAGE_ISTORAGEACCESSOR_H_
#define HUESTREAM_COMMON_STORAGE_ISTORAGEACCESSOR_H_

#include "huestream/connect/OperationResult.h"
#include "huestream/common/serialize/Serializable.h"

#include <memory>
#include <functional>

namespace huestream {

    typedef std::function<void(OperationResult, SerializablePtr)> LoadCallbackHandler;
    typedef std::function<void(OperationResult)> SaveCallbackHandler;


    class IStorageAccessor {
    public:
        virtual ~IStorageAccessor() {}

        virtual void Load(LoadCallbackHandler cb) = 0;

        virtual void Save(SerializablePtr, SaveCallbackHandler cb) = 0;
    };

    typedef std::shared_ptr<IStorageAccessor> StorageAccessorPtr;

}  // namespace huestream

#endif  // HUESTREAM_COMMON_STORAGE_ISTORAGEACCESSOR_H_
