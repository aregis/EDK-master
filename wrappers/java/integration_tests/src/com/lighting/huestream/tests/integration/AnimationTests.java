package com.lighting.huestream.tests.integration;

import com.lighting.huestream.*;
import com.lighting.huestream.tests.integration.helpers.BridgeWrapperBuilder;
import com.lighting.huestream.tests.integration.helpers.IBridgeWrapper;
import com.lighting.huestream.tests.integration.helpers.Light;
import org.hamcrest.CoreMatchers;
import org.junit.*;
import org.junit.rules.ErrorCollector;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.stream.Collectors;
import java.util.function.BiConsumer;
import java.util.function.Function;

public class AnimationTests extends BaseTest {

    private static String RGB_BLACK = "#000000";
    private static long TIMEOUT_MS = 1000;



    @Rule
    public ErrorCollector _errorCollector = new ErrorCollector();

    @BeforeClass
    public static void loadNativeLibrary() {
        System.loadLibrary("huestream_java_native");
    }

    @Before
    public void setUp() {
        _hue_stream = createStream(StreamingMode.STREAMING_MODE_DTLS);
        _hue_stream.RegisterFeedbackHandler(new FeedBackHandler());

        _bridge = initializeBridge();
        _bridgeWrapperHelper = initializeBridgeWrapper();
        Assert.assertNotNull("_bridgeWrapperHelper is NULL", _bridgeWrapperHelper);
        initializeBridgeResources();

        _hue_stream.ConnectManualBridgeInfo(_bridge);

        _allLights.forEach(light -> expectSameColor(RGB_BLACK, light));
    }

    @After
    public void tearDown() {
        _hue_stream.ShutDown();
    }

    private void threadWaitFor(int milliseconds) {
        try {
            Thread.sleep(milliseconds);
        } catch (InterruptedException e) {
            Assert.fail("Thread was interrupted when sleeping");
        }
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

    private void expectColor(Function<String, Boolean> successCondition, BiConsumer<String, Long>  after, Light... lights) {
        Assert.assertNotEquals("Incoming lights collection is empty", 0, lights.length);
        long startedAt = System.currentTimeMillis();
        for (Light light : lights) {
            String actualColor = _bridgeWrapperHelper.getLightRBGColor(light.asLightID());
            long diffMs = System.currentTimeMillis() - startedAt;
            while (!successCondition.apply(actualColor) && diffMs < TIMEOUT_MS) {
                threadWaitFor(10);
                actualColor = _bridgeWrapperHelper.getLightRBGColor(light.asLightID());
                diffMs = System.currentTimeMillis() - startedAt;
            }
            after.accept(actualColor, diffMs);
        }
    }

    private void expectSameColor(final String expectedColor, Light... lights) {
        expectColor(
            (actualColor) -> actualColor.equals(expectedColor),
            (actualColor, timeout) -> _errorCollector.checkThat(String.format("Colors are not equal, expected=%s, actual %s, timeout %d ms", expectedColor, actualColor, timeout),
                                      actualColor, CoreMatchers.equalTo(expectedColor)),
            lights);
    }

    private void expectDifferentColor(final String expectedColor, Light... lights) {
        expectColor(
            (actualColor) -> !actualColor.equals(expectedColor),
            (actualColor, timeout) -> _errorCollector.checkThat(String.format("Colors are equal, not expected=%s, actual %s, timeout %d ms", expectedColor, actualColor, timeout),
                                      actualColor, CoreMatchers.not(expectedColor)),
            lights);
    }

    @Test
    @Ignore // see HSDK-2273
    public void simpleOverallAreaEffect() {
        final AreaEffect effect = new AreaEffect("simpleAreaEffect", 0);
        effect.AddArea(Area.getAll());
        final Color effectColor = new Color(0.0, 1.0, 0.0);
        effect.SetFixedColor(effectColor);

        addEffectToEngine(effect);
        effect.Enable();

        _allLights.forEach(light -> expectDifferentColor(RGB_BLACK, light));
    }

    @Test
    @Ignore // see HSDK-2273
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

        expectDifferentColor(RGB_BLACK, _rearLeftLight, _frontRightLight);
        expectSameColor(RGB_BLACK, _rearRightLight, _frontLeftLight);
    }

    @Test
    @Ignore // see HSDK-2273
    public void lightSourceEffect() {
        final LightSourceEffect effect = new LightSourceEffect();
        effect.SetColorAnimation(new ConstantAnimation(0.0)
                , new ConstantAnimation(1.)
                , new ConstantAnimation(0.0));
        effect.SetPositionAnimation(new ConstantAnimation(0), new ConstantAnimation(0));
        effect.SetRadius(new ConstantAnimation(1.5));

        addEffectToEngine(effect);
        effect.Enable();

        _allLights.forEach(light -> expectDifferentColor(RGB_BLACK, light));
    }

    @Test
    @Ignore // see HSDK-2273
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

        expectDifferentColor(RGB_BLACK, _rearRightLight);
        expectSameColor(RGB_BLACK, _frontRightLight, _frontLeftLight, _rearLeftLight);

        threadWaitFor(OFFSET_DIRATION);
        expectDifferentColor(RGB_BLACK, _rearLeftLight);
        expectSameColor(RGB_BLACK, _rearRightLight, _frontRightLight, _frontLeftLight);


        threadWaitFor(OFFSET_DIRATION);
        expectDifferentColor(RGB_BLACK, _frontLeftLight);
        expectSameColor(RGB_BLACK, _frontRightLight, _rearRightLight, _rearLeftLight);

        threadWaitFor(OFFSET_DIRATION);
        expectDifferentColor(RGB_BLACK, _frontRightLight);
        expectSameColor(RGB_BLACK, _frontLeftLight, _rearLeftLight, _rearRightLight);
    }

    @Test
    @Ignore // see HSDK-2273
    public void manualEffect() {
        final ManualEffect effect = new ManualEffect();

        effect.SetIdToColor(_frontLeftLight.getLightId().toString(), new Color(1.0, 0.0, 0.0));
        effect.SetIdToColor(_frontRightLight.getLightId().toString(), new Color(0.0, 1.0, 0.0));
        effect.SetIdToColor(_rearLeftLight.getLightId().toString(), new Color(0.0, 0.0, 1.0));
        effect.SetIdToColor(_rearRightLight.getLightId().toString(), new Color(1.0, 1.0, 0.0));

        addEffectToEngine(effect);
        effect.Enable();

        _allLights.forEach(light -> expectDifferentColor(RGB_BLACK, light));
    }
}
