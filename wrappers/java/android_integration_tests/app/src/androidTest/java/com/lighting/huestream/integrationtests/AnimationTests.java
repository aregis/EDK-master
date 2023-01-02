package com.lighting.huestream.integrationtests;

import android.util.Log;

import com.lighting.huestream.Area;
import com.lighting.huestream.AreaEffect;
import com.lighting.huestream.Color;
import com.lighting.huestream.ConstantAnimation;
import com.lighting.huestream.Effect;
import com.lighting.huestream.IFeedbackMessageHandler;
import com.lighting.huestream.IterationMode;
import com.lighting.huestream.IterationOrder;
import com.lighting.huestream.LightIteratorEffect;
import com.lighting.huestream.LightSourceEffect;
import com.lighting.huestream.ManualEffect;
import com.lighting.huestream.StreamingMode;
import com.lighting.huestream.TweenAnimation;
import com.lighting.huestream.TweenType;
import com.lighting.huestream.integrationtests.helpers.IBridgeWrapper;
import com.lighting.huestream.integrationtests.helpers.Light;

import org.hamcrest.CoreMatchers;
import org.junit.After;
import org.junit.Assert;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Ignore;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.ErrorCollector;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

import java.util.ArrayList;
import java.util.List;

@RunWith(JUnit4.class)
public class AnimationTests extends BaseTest {

    private static final String RGB_BLACK = "#000000";
    private static final long TIMEOUT_MS = 1000;

    private IFeedbackMessageHandler _messageHandler = new FeedBackHandler();

    @Rule
    public ErrorCollector _errorCollector = new ErrorCollector();

    @BeforeClass
    public static void loadNativeLibrary() {
        Log.d("IntegrationTests", "Loading native library");
        System.loadLibrary("huestream_java_native");
        Log.d("IntegrationTests", "Loaded native library");
    }

    @Before
    public void setUp() {
        _hue_stream = createStream(StreamingMode.STREAMING_MODE_DTLS);
        _hue_stream.RegisterFeedbackHandler(_messageHandler);

        _bridge = initializeBridge();
        _bridgeWrapperHelper = initializeBridgeWrapper();
        Assert.assertNotNull("_bridgeWrapperHelper is NULL", _bridgeWrapperHelper);
        initializeBridgeResources();

        _hue_stream.ConnectManualBridgeInfo(_bridge);

        threadWaitFor(100);
        expectBlack(_frontLeftLight, _frontRightLight, _rearLeftLight, _rearRightLight);
    }

    @After
    public void tearDown() {
        threadWaitFor(1000);
        _hue_stream.ShutDown();
    }

    private void addEffectToEngine(Effect... effects) {
        _hue_stream.LockMixer();

        try {
            for (int i = 0; i < effects.length; ++i) {
                _hue_stream.AddEffect(effects[i]);
            }
        } catch (Exception e) {
            Assert.fail("Could not add effect to the engine");
        }

        _hue_stream.UnlockMixer();
    }

    private enum ExpectedCondition {
        Equal,
        Different
    }

    private boolean conditionMet(final String expectedColor, final String actualColor, final ExpectedCondition condition) {
        if (condition == ExpectedCondition.Equal) {
            return actualColor.equals(expectedColor);
        }
        return !actualColor.equals(expectedColor);
    }

    private void checkThat(final String expectedColor, final String actualColor, final ExpectedCondition condition, final long timeout) {
        if (condition == ExpectedCondition.Equal) {
            _errorCollector.checkThat(String.format("Colors are not equal, expected=%s, actual %s", expectedColor, actualColor, timeout),
                    actualColor, CoreMatchers.equalTo(expectedColor));
        } else {
            _errorCollector.checkThat(String.format("Colors are equal, not expected=%s, actual %s", expectedColor, actualColor, timeout),
                    actualColor, CoreMatchers.not(expectedColor));
        }
    }

    private void expectColor(final String expectedColor, final ExpectedCondition expectedCondition, Light... lights) {
        Assert.assertNotEquals("Incoming lights collection is empty", 0, lights.length);
        long startedAt = System.currentTimeMillis();
        for (Light light : lights) {
            String actualColor = _bridgeWrapperHelper.getLightRBGColor(light.asLightID());
            long diffMs = System.currentTimeMillis() - startedAt;
            while (!conditionMet(expectedColor, actualColor, expectedCondition) && diffMs < TIMEOUT_MS) {
                threadWaitFor(10);
                actualColor = _bridgeWrapperHelper.getLightRBGColor(light.asLightID());
                diffMs = System.currentTimeMillis() - startedAt;
            }
            checkThat(expectedColor, actualColor, expectedCondition, diffMs);
        }
    }

    private void expectBlack(Light... lights) {
        expectColor(RGB_BLACK, ExpectedCondition.Equal, lights);
    }

    private void expectNotBlack(Light... lights) {
        expectColor(RGB_BLACK, ExpectedCondition.Different, lights);
    }

    @Test
    @Ignore("HSDK-2347")
    public void simpleOverallAreaEffect() {
        final AreaEffect effect = new AreaEffect("simpleAreaEffect", 0);
        effect.AddArea(Area.getAll());
        final Color effectColor = new Color(0.0, 1.0, 0.0);
        effect.SetFixedColor(effectColor);

        addEffectToEngine(effect);
        effect.Enable();

        threadWaitFor(100);
        expectNotBlack(_frontRightLight, _frontLeftLight, _rearLeftLight, _rearRightLight);
    }

    @Test
    @Ignore("HSDK-2347")
    public void twoAreaEffect() {
        final AreaEffect rearLeftPart = new AreaEffect("rearLeft", 1);
        rearLeftPart.AddArea(Area.getBackLeft());
        final Color rearLeftColor = new Color(0.0, 1.0, 0.0);
        rearLeftPart.SetFixedColor(rearLeftColor);

        final AreaEffect frontRightPart = new AreaEffect("frontRight", 2);
        frontRightPart.AddArea(Area.getFrontRight());
        final Color frontRightColor = new Color(1.0, 1.0, 1.0);
        frontRightPart.SetFixedColor(frontRightColor);

        addEffectToEngine(frontRightPart);
        addEffectToEngine(rearLeftPart);
        rearLeftPart.Enable();
        frontRightPart.Enable();

        threadWaitFor(200);
        expectNotBlack(_rearLeftLight, _frontRightLight);
        expectBlack(_rearRightLight, _frontLeftLight);
    }

    @Test
    @Ignore("HSDK-2347")
    public void lightSourceEffect() {
        final LightSourceEffect effect = new LightSourceEffect();
        effect.SetColorAnimation(new ConstantAnimation(0.0)
                , new ConstantAnimation(1.)
                , new ConstantAnimation(0.0));
        effect.SetPositionAnimation(new ConstantAnimation(0), new ConstantAnimation(0));
        effect.SetRadius(new ConstantAnimation(1.5));

        addEffectToEngine(effect);
        effect.Enable();

        threadWaitFor(100);

        expectNotBlack(_frontLeftLight, _frontRightLight, _rearLeftLight, _rearRightLight);
    }

    @Test
    @Ignore("HSDK-2347")
    public void lightIteratorEffect() {
        int OFFSET_DIRATION = 1000;

        final LightIteratorEffect effect = new LightIteratorEffect();
        effect.SetColorAnimation(new TweenAnimation(0, 0, OFFSET_DIRATION, TweenType.Linear)
                , new TweenAnimation(1, 1, OFFSET_DIRATION, TweenType.Linear)
                , new TweenAnimation(0, 0, OFFSET_DIRATION, TweenType.Linear));

        effect.SetOpacityAnimation(new TweenAnimation(1,1, OFFSET_DIRATION, TweenType.Linear));
        effect.SetOffset(OFFSET_DIRATION);
        effect.SetOrder(IterationOrder.IterationOrderClockwise);
        effect.setMode(IterationMode.IterationModeCycle);

        addEffectToEngine(effect);
        effect.Enable();

        threadWaitFor(100);
        expectNotBlack(_rearRightLight);
        expectBlack(_frontRightLight, _frontLeftLight, _rearLeftLight);

        threadWaitFor(OFFSET_DIRATION);
        expectNotBlack(_rearLeftLight);
        expectBlack(_rearRightLight, _frontRightLight, _frontLeftLight);


        threadWaitFor(OFFSET_DIRATION);
        expectNotBlack(_frontLeftLight);
        expectBlack(_frontRightLight, _rearRightLight, _rearLeftLight);

        threadWaitFor(OFFSET_DIRATION);
        expectNotBlack(_frontRightLight);
        expectBlack(_frontLeftLight, _rearLeftLight, _rearRightLight);
    }

    @Test
    @Ignore("HSDK-2347")
    public void manualEffect() {
        final ManualEffect effect = new ManualEffect();

        effect.SetIdToColor(_frontLeftLight.getLightId().toString(), new Color(1.0, 0.0, 0.0));
        effect.SetIdToColor(_frontRightLight.getLightId().toString(), new Color(0.0, 1.0, 0.0));
        effect.SetIdToColor(_rearLeftLight.getLightId().toString(), new Color(0.0, 0.0, 1.0));
        effect.SetIdToColor(_rearRightLight.getLightId().toString(), new Color(1.0, 1.0, 0.0));

        addEffectToEngine(effect);
        effect.Enable();

        threadWaitFor(100);

        expectNotBlack(_frontLeftLight, _frontRightLight, _rearLeftLight, _rearRightLight);
    }
}
