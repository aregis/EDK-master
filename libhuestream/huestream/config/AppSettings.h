/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/
/** @file */

#ifndef HUESTREAM_CONFIG_APPSETTINGS_H_
#define HUESTREAM_CONFIG_APPSETTINGS_H_

#include "huestream/common/serialize/SerializerHelper.h"
#include "huestream/stream/ProtocolSerializer.h"

#include <memory>
#include <string>

namespace huestream {

    typedef enum {
        ACTIVATION_OVERRIDELEVEL_NEVER = 0x00,
        ACTIVATION_OVERRIDELEVEL_SAMEGROUP = 0x01,
        ACTIVATION_OVERRIDELEVEL_ALWAYS = 0x02
    } ActivationOverrideLevel;

    /**
     various application configuration settings for the library
     */
    class AppSettings {
    public:
        static constexpr const char* BRIDGE_STORAGE_FILENAME = "bridge.json";

        AppSettings();

        bool UseForcedActivation();

    /**
     set which stream activation override level is used which means the library will override another application already streaming
     @note default SAMEGROUP: only override if other app is streaming to same group
     @note should be NEVER for non content specific applications to give content specific applications priority
     @note should only be ALWAYS if the application has a built in warning that action will override session in other room
     */
    PROP_DEFINE(AppSettings, ActivationOverrideLevel, activationOverride, ActivationOverride);

    /**
     set whether the library will automatically start streaming (controlling the lights) right after the bridge has been connected to
     @note default true, should be false when application wants to start controlling lights on another moment than right after connection
     */
    PROP_DEFINE_BOOL(AppSettings, bool, autoStartAtConnection, AutoStartAtConnection);

    /**
     set whether the library runs its own renderthread
     @note default true, should be false when the application wants to control the render loop manually
     */
    PROP_DEFINE_BOOL(AppSettings, bool, useRenderThread, UseRenderThread);

    /**
     set a user friendly name of the application
     @note no default, this is mandatory to set
     */
    PROP_DEFINE(AppSettings, std::string, appName, AppName);

    /**
     set a user friendly name of the platfrom this application is running on
     @note no default, this is mandatory to set
     */
    PROP_DEFINE(AppSettings, std::string, deviceName, DeviceName);

    /**
     set the ISO 639-1 language code used by the library to output user messages in the correct language
     @note default en
     */
    PROP_DEFINE(AppSettings, std::string, language, Language);

    /**
     set the ISO 3166-1 alpha-2 country CODE used by the library to output user messages in the correct language
     @note default US
     */
    PROP_DEFINE(AppSettings, std::string, region, Region);

    /**
     set where the bridge information is stored with full path (absolute or relative) and file name
     @note default bridge.json (next to executable), needs to be a writable location for the platform
     */
    PROP_DEFINE(AppSettings, std::string, bridgeFileName, BridgeFileName);

    /**
     set the bridge monitoring polling interval during streaming
     @note default 5000 (5 seconds)
     */
    PROP_DEFINE(AppSettings, int, monitorIntervalStreamingMs, MonitorIntervalStreamingMs);

    /**
     set the bridge monitoring polling interval when not streaming
     @note default 15000 (15 seconds)
     */
    PROP_DEFINE(AppSettings, int, monitorIntervalNotStreamingMs, MonitorIntervalNotStreamingMs);

		/**
		set the bridge connection monitoring polling interval when clipV2 is used
		@note default 15000 (15 seconds)
		*/
		PROP_DEFINE(AppSettings, int, monitorIntervalConnectionMs, MonitorIntervalConnectionMs);

    /**
     set the encryption key used to encrypt stored bridge information
     */
    PROP_DEFINE(AppSettings, std::string, storageEncryptionKey, StorageEncryptionKey);

    /**
     set whether the lights retain their last color when no more effects are applied to them
     @note default false, which means the light turns off when all effects are finished
     */
    PROP_DEFINE_BOOL(AppSettings, bool, lightsRetainColor, LightsRetainColor);
    };

    /**
     shared pointer to a huestream::AppSettings object
     */
    typedef std::shared_ptr<AppSettings> AppSettingsPtr;
}  // namespace huestream

#endif  // HUESTREAM_CONFIG_APPSETTINGS_H_
