/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#ifndef HUESTREAM_COMMON_STORAGE_FILESTORAGEACCESSOR_H_
#define HUESTREAM_COMMON_STORAGE_FILESTORAGEACCESSOR_H_

#include "huestream/common/storage/IStorageAccessor.h"

#include <string>

using huestream::IStorageAccessor;
using huestream::LoadCallbackHandler;
using huestream::SaveCallbackHandler;

namespace huestream {

    class FileStorageAccessor : public IStorageAccessor {
    public:
        explicit FileStorageAccessor(const std::string &fileName);

        void Load(LoadCallbackHandler cb) override;

        void Save(SerializablePtr serializable, SaveCallbackHandler cb) override;

    private:
        std::string _fileName;
    };

}  // namespace huestream

#endif  // HUESTREAM_COMMON_STORAGE_FILESTORAGEACCESSOR_H_
