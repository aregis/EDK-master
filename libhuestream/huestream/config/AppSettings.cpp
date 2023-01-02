/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <huestream/config/AppSettings.h>

#include <string>

namespace huestream {

    PROP_IMPL(AppSettings, ActivationOverrideLevel, activationOverride, ActivationOverride);
    PROP_IMPL_BOOL(AppSettings, bool, autoStartAtConnection, AutoStartAtConnection);
    PROP_IMPL_BOOL(AppSettings, bool, useRenderThread, UseRenderThread);
    PROP_IMPL(AppSettings, std::string, appName, AppName);
    PROP_IMPL(AppSettings, std::string, deviceName, DeviceName);
    PROP_IMPL(AppSettings, std::string, language, Language);
    PROP_IMPL(AppSettings, std::string, region, Region);
    PROP_IMPL(AppSettings, std::string, bridgeFileName, BridgeFileName);
    PROP_IMPL(AppSettings, int, monitorIntervalStreamingMs, MonitorIntervalStreamingMs);
    PROP_IMPL(AppSettings, int, monitorIntervalNotStreamingMs, MonitorIntervalNotStreamingMs);
		PROP_IMPL(AppSettings, int, monitorIntervalConnectionMs, MonitorIntervalConnectionMs);
    PROP_IMPL(AppSettings, std::string, storageEncryptionKey, StorageEncryptionKey);
    PROP_IMPL_BOOL(AppSettings, bool, lightsRetainColor, LightsRetainColor);

    AppSettings::AppSettings() {
        SetActivationOverride(ACTIVATION_OVERRIDELEVEL_SAMEGROUP);
        SetAutoStartAtConnection(true);
        SetUseRenderThread(true);
        SetAppName("");
        SetDeviceName("");
        SetLanguage("en");
        SetRegion("US");
        SetBridgeFileName(BRIDGE_STORAGE_FILENAME);
        SetMonitorIntervalStreamingMs(5000);
        SetMonitorIntervalNotStreamingMs(15000);
				SetMonitorIntervalConnectionMs(15000);
        SetLightsRetainColor(false);
    }

    bool AppSettings::UseForcedActivation() {
        return _activationOverride != ACTIVATION_OVERRIDELEVEL_NEVER;
    }
}  // namespace huestream
