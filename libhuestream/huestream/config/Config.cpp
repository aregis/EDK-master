/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <huestream/config/Config.h>

#include <memory>
#include <string>

using std::make_shared;

namespace huestream {

    Config::Config(const std::string &appName,
                              const std::string &deviceName,
                              const PersistenceEncryptionKey &persistenceEncryptionKey,
                              const std::string &language,
                              const std::string &region) :
            _bridgeSettings(std::make_shared<BridgeSettings>()),
            _streamSettings(std::make_shared<StreamSettings>()),
            _appSettings(std::make_shared<AppSettings>()),
            _streamingMode(STREAMING_MODE_DTLS) {
        _appSettings->SetAppName(appName);
        _appSettings->SetDeviceName(deviceName);
        _appSettings->SetLanguage(language);
        _appSettings->SetRegion(region);
        _appSettings->SetStorageEncryptionKey(persistenceEncryptionKey.data);
    }

    Config::~Config() {
        _bridgeSettings = nullptr;
        _streamSettings = nullptr;
        _appSettings = nullptr;
    }

    PROP_IMPL(Config, BridgeSettingsPtr, bridgeSettings, BridgeSettings);
    PROP_IMPL(Config, StreamSettingsPtr, streamSettings, StreamSettings);
    PROP_IMPL(Config, AppSettingsPtr, appSettings, AppSettings);
    PROP_IMPL(Config, StreamingMode, streamingMode, StreamingMode);

}  // namespace huestream
