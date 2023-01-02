/*******************************************************************************
 Copyright (C) 2021 Signify Holding B.V.
 All Rights Reserved.
 *******************************************************************************/

package com.signify.android.hueexample;

import android.content.Context;
import android.content.SharedPreferences;
import android.os.Build;
import android.util.Log;

import com.lighting.huestream.Bridge;
import com.lighting.huestream.BridgeStatus;
import com.lighting.huestream.Config;
import com.lighting.huestream.ConnectResult;
import com.lighting.huestream.DummyTranslator;
import com.lighting.huestream.FeedbackMessage;
import com.lighting.huestream.Group;
import com.lighting.huestream.HueStream;
import com.lighting.huestream.IFeedbackMessageHandler;
import com.lighting.huestream.PersistenceEncryptionKey;
import com.lighting.huestream.AreaEffect;
import com.lighting.huestream.Area;
import com.lighting.huestream.Color;
import com.lighting.huestream.ConstantAnimation;
import com.lighting.huestream.PointVector;
import com.lighting.huestream.Point;
import com.lighting.huestream.CurveAnimation;
import com.lighting.huestream.LightSourceEffect;
import com.lighting.huestream.ExplosionEffect;
import com.lighting.huestream.Location;
import com.philips.lighting.hue.sdk.wrapper.utilities.InitSdk;

import java.util.ArrayList;
import java.util.List;

public class HueHandler extends IFeedbackMessageHandler {
    private static final String LOG_TAG = "HueHandler";

    private HueStream hueStream;
    private List<HueFeedbackListener> listeners = new ArrayList<>();
    private Context ctx;

    private AreaEffect greenEffect = null;
    private AreaEffect leftEffect = null;
    private LightSourceEffect rightEffect = null;
    private ExplosionEffect explosionEffect = null;

    public HueHandler(Context appCtx, String appName) {
        InitSdk.setApplicationContext(appCtx);
        ctx = appCtx;

        Config config = new Config(appName, getDeviceName(), new PersistenceEncryptionKey("nfDk3l7rMw9c"));
        config.GetAppSettings().SetBridgeFileName(appCtx.getFilesDir() + "/huebridges.json");
        config.GetAppSettings().SetAutoStartAtConnection(false);
        hueStream = new HueStream(config);
        hueStream.RegisterFeedbackHandler(this);

        hueStream.ConnectBridgeBackgroundAsync();
    }

    @Override
    public void NewFeedbackMessage(FeedbackMessage message) {
        Log.d(LOG_TAG, message.GetDebugMessage());
        for (HueFeedbackListener listener : listeners) {
            listener.onFeedbackMessage(message);
        }
    }

    public boolean isSetup() {
        return hueStream.GetLoadedBridgeStatus() != BridgeStatus.BRIDGE_EMPTY;
    }

    public boolean isConnected() {
        return hueStream.GetLoadedBridge().IsConnected();
    }

    public String getSetupText() {
        ConnectResult r = getConnectionResult();

        String text = ctx.getResources().getString(R.string.hue_setup_general);
        if (r == ConnectResult.BridgeFound) {
            text = ctx.getResources().getString(R.string.hue_setup_owner);
        } else if (r == ConnectResult.ActionRequired) {
            //text = getStatusText(ctx.getResources().getConfiguration().locale.getLanguage());
            text = getStatusText("en");
        } else if (r == ConnectResult.ReadyToStart) {
            text = ctx.getResources().getString(R.string.hue_setup_connected) + getBridge().GetName();
        } else if (r == ConnectResult.Streaming) {
            text = ctx.getResources().getString(R.string.hue_setup_syncing) + getBridge().GetName();
        }

        return text;
    }

    private String getStatusText(String lang) {
        return getBridge().GetUserStatus(new DummyTranslator(lang));
    }

    public void connect() {
        hueStream.ConnectBridgeAsync();
    }

    public void connectWithIp(String ip) {
        hueStream.ConnectBridgeManualIpAsync(ip);
    }

    public void connectNewBridge() {
        hueStream.ConnectNewBridgeAsync();
    }

    public void abortConnecting() {
        hueStream.AbortConnecting();
    }

    public ConnectResult getConnectionResult() {
        return hueStream.GetConnectionResult();
    }

    public void selectGroup(Group group) {
        hueStream.SelectGroupAsync(group);
    }

    public Bridge getBridge() {
        return hueStream.GetLoadedBridge();
    }

    public void registerFeedbackListener(HueFeedbackListener listener) {
        listeners.add(listener);
    }

    public void deregisterFeedbackListener(HueFeedbackListener listener) {
        listeners.remove(listener);
    }

    public void stopStreaming() {
        hueStream.Stop();
    }

    public void startStreaming() {
        if (!hueStream.IsBridgeStreaming()) {
            hueStream.Start();
        }
    }

    public void reset() {
        hueStream.ResetAllPersistentDataAsync();
    }

    public void setLightsToGreen() {
        startStreaming();
        hueStream.LockMixer();
        if (greenEffect != null) {
            greenEffect.Finish();
        }
        if (leftEffect != null) {
            leftEffect.Finish();
        }
        if (rightEffect != null) {
            rightEffect.Finish();
        }
        if (explosionEffect != null) {
            explosionEffect.Finish();
        }

        greenEffect = new AreaEffect("", 0);
        greenEffect.AddArea(Area.getAll());
        greenEffect.SetFixedColor(new Color(0.0,1.0,0.0));
        greenEffect.Enable();
        hueStream.AddEffect(greenEffect);
        hueStream.UnlockMixer();
    }

    public void showAnimatedLightsExample() {
        startStreaming();
        hueStream.LockMixer();
        if (greenEffect != null) {
            greenEffect.Finish();
        }
        if (leftEffect != null) {
            leftEffect.Finish();
        }
        if (rightEffect != null) {
            rightEffect.Finish();
        }
        if (explosionEffect != null) {
            explosionEffect.Finish();
        }

        //Create an animation which is fixed 0
        ConstantAnimation fixedZero = new ConstantAnimation(0.0);

        //Create an animation which is fixed 1
        ConstantAnimation fixedOne = new ConstantAnimation(1.0);

        //Create an animation which repeats a 2 second sawTooth 5 times
        PointVector pointList = new PointVector();
        pointList.add(new Point(   0, 0.0));
        pointList.add(new Point(1000, 1.0));
        pointList.add(new Point(2000, 0.0));
        double repeatTimes = 5;
        CurveAnimation sawTooth = new CurveAnimation(repeatTimes, pointList);

        //Create an effect on the left half of the room where blue is animated by sawTooth
        leftEffect = new AreaEffect("LeftArea", 1);
        leftEffect.AddArea(Area.getLeftHalf());
        leftEffect.SetColorAnimation(fixedZero, fixedZero, sawTooth);

        //Create a red virtual light source where x-axis position is animated by sawTooth
        rightEffect = new LightSourceEffect("RightSource", 1);
        rightEffect.SetFixedColor(new Color(1.0,0.0,0.0));
        rightEffect.SetPositionAnimation(sawTooth, fixedZero);
        rightEffect.SetRadiusAnimation(fixedOne);

        //Create effect from predefined explosionEffect
        explosionEffect = new ExplosionEffect("explosion", 2);
        Color explosionColorRGB = new Color(1.0, 0.8, 0.4);
        Location explosionLocationXY = new Location(0, 1);
        double radius = 2.0;
        double duration_ms = 2000;
        double expAlpha_ms = 50;
        double expRadius_ms = 100;
        explosionEffect.PrepareEffect(explosionColorRGB, explosionLocationXY, duration_ms, radius, expAlpha_ms, expRadius_ms);

        //Now play all effects
        hueStream.AddEffect(leftEffect);
        hueStream.AddEffect(rightEffect);
        hueStream.AddEffect(explosionEffect);
        leftEffect.Enable();
        rightEffect.Enable();
        explosionEffect.Enable();
        hueStream.UnlockMixer();
    }

    public interface HueFeedbackListener {
        void onFeedbackMessage(final FeedbackMessage message);
    }

    private String getDeviceName() {
        String manufacturer = Build.MANUFACTURER;
        String model = Build.MODEL;
        if (model.toLowerCase().startsWith(manufacturer.toLowerCase())) {
            return capitalize(model);
        } else {
            return capitalize(manufacturer) + " " + model;
        }
    }

    private String capitalize(String s) {
        if (s == null || s.length() == 0) {
            return "";
        }
        char first = s.charAt(0);
        if (Character.isUpperCase(first)) {
            return s;
        } else {
            return Character.toUpperCase(first) + s.substring(1);
        }
    }
}
