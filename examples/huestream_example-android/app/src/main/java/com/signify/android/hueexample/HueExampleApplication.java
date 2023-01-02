/*******************************************************************************
 Copyright (C) 2021 Signify Holding B.V.
 All Rights Reserved.
 *******************************************************************************/

package com.signify.android.hueexample;

import android.app.Application;

public class HueExampleApplication extends Application {

    static final String appName = "Dummy Example";

    private HueHandler hue;

    static {
        try {
            System.loadLibrary("huestream_java_native");
        } catch (UnsatisfiedLinkError e) {
            e.printStackTrace();
        }
    }

    @Override
    public void onCreate() {
        super.onCreate();
        hue = new HueHandler(getApplicationContext(), appName);
    }

    public HueHandler getHue() {
        return hue;
    }
}
