/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <huestream/common/data/BridgeSettings.h>

namespace huestream {

    PROP_IMPL(BridgeSettings, int, supportedApiVersionMajor, SupportedApiVersionMajor);
    PROP_IMPL(BridgeSettings, int, supportedApiVersionMinor, SupportedApiVersionMinor);
    PROP_IMPL(BridgeSettings, int, supportedApiVersionBuild, SupportedApiVersionBuild);
    PROP_IMPL(BridgeSettings, int, supportedModel, SupportedModel);

    PROP_IMPL(BridgeSettings, int, supportedClipV2SwVersion, SupportedClipV2SwVersion);

    BridgeSettings::BridgeSettings() {
        SetSupportedApiVersionMajor(1);
        SetSupportedApiVersionMinor(24);
        SetSupportedApiVersionBuild(0);
        SetSupportedModel(2);

        SetSupportedClipV2SwVersion(1944193080);
    }
}  // namespace huestream
