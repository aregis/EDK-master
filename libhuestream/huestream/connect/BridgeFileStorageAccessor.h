/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#ifndef HUESTREAM_CONNECT_BRIDGEFILESTORAGEACCESSOR_H_
#define HUESTREAM_CONNECT_BRIDGEFILESTORAGEACCESSOR_H_

#include "huestream/connect/IBridgeStorageAccessor.h"

#include <string>

#include "huestream/config/AppSettings.h"

namespace huestream {

    class BridgeFileStorageAccessor : public IBridgeStorageAccessor {
    public:
        BridgeFileStorageAccessor(const std::string &fileName, AppSettingsPtr appSettings, BridgeSettingsPtr hueSettings);

        void Load(BridgesLoadCallbackHandler cb) override;

        void Save(HueStreamDataPtr bridges, BridgesSaveCallbackHandler cb) override;

    private:
        std::string get_encryption_key_hash();
        std::string encrypt_data(const std::string& data);
        std::string decrypt_data(const std::string& data);

        std::string _fileName;
        AppSettingsPtr _appSettings;
        BridgeSettingsPtr _bridgeSettings;
    };

}  // namespace huestream

#endif  // HUESTREAM_CONNECT_BRIDGEFILESTORAGEACCESSOR_H_
