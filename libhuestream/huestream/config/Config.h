/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/
/** @file */

#ifndef HUESTREAM_CONFIG_CONFIG_H_
#define HUESTREAM_CONFIG_CONFIG_H_

#include "huestream/common/data/BridgeSettings.h"
#include "huestream/stream/StreamSettings.h"
#include "huestream/config/AppSettings.h"

#include <string>
#include <memory>

namespace huestream {

    /**
     available streaming modes
     */
    typedef enum {
        STREAMING_MODE_DTLS = 0,    ///< default secure streaming mode used for actual bridges
        STREAMING_MODE_UDP,         ///< unsecure streaming mode only used for simulator or debugging
    } StreamingMode;

    /**
     Encryption key wrapper.
     */
    struct PersistenceEncryptionKey {
        explicit PersistenceEncryptionKey(const std::string& key) : data(key) {}
        std::string data;
    };

    /**
     configuration settings for the library
     @param appName Name by which users can clearly recognize this application, must be max 20 characters and must not contain a '#'
     @param deviceName Name of device which this application instance is running on, either user configured name or model name
     @param persistenceEncryptionKey Key which is used to encrypt bridge storage
     */
    class Config {
    public:
        Config(const std::string &appName,
                  const std::string &deviceName,
                  const PersistenceEncryptionKey& persistenceEncryptionKey,
                  const std::string &language = "en",
                  const std::string &region = "US");

        ~Config();

    /**
     configure bridge settings
     @note mainly internal use
     */
    PROP_DEFINE(Config, BridgeSettingsPtr, bridgeSettings, BridgeSettings);

    /**
     configure the streaming protocol
     */
    PROP_DEFINE(Config, StreamSettingsPtr, streamSettings, StreamSettings);

    /**
     configure application settings
     */
    PROP_DEFINE(Config, AppSettingsPtr, appSettings, AppSettings);

    /**
     configure streaming mode
     */
    PROP_DEFINE(Config, StreamingMode, streamingMode, StreamingMode);
    };
    typedef std::shared_ptr<Config> ConfigPtr;

}  // namespace huestream

#endif  // HUESTREAM_CONFIG_CONFIG_H_
