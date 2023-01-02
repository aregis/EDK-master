package com.philips.lighting.hue.sdk.wrapper.utilities;

import com.philips.lighting.hue.sdk.wrapper.utilities.InitSdk;
import com.philips.lighting.hue.sdk.wrapper.utilities.WifiUtil;

public class WifiUtilFactory {
    @SuppressWarnings("unused")
    private static WifiUtil getWifiUtil() {
        return new WifiUtil(InitSdk.getApplicationContext());
    }
}
