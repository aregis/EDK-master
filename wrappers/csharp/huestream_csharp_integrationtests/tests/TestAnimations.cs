using NUnit.Framework;
using System;
using huestream;
using System.Threading;
using System.Collections.Generic;
using System.Linq;

namespace huestream_tests
{
    [TestFixture()]
    public class TestAnimations : TestBase
    {
        private const String RGB_BLACK = "#000000";
        private const double TIMEOUT_MS = 1000;

        [SetUp]
        public void SetUp()
        {
            _message_handler = new FeedbackMessageHandler();

            _hue_stream = CreateStream(StreamingMode.STREAMING_MODE_DTLS);
            _hue_stream.RegisterFeedbackHandler(_message_handler);

            _bridge = InitializeBridge();
            _bridgeWrapperHelper = InitializeBridgeWrapper();
            Assert.NotNull(_bridgeWrapperHelper, "_bridgeWrapperHelper is NULL");
            InitializeBridgeResources();

            _hue_stream.ConnectManualBridgeInfo(_bridge);

            _allLights.ForEach(light => AssertSameColor(RGB_BLACK, light));
        }

        [TearDown]
        public void TearDown()
        {
            _hue_stream.ShutDown();
        }

        private void ThreadWaitFor(int milliseconds)
        {
            Thread.Sleep(milliseconds);
        }

        private void AddEffectToEngine(params Effect[] effects)
        {
            _hue_stream.LockMixer();

            try
            {
                for (int i = 0; i < effects.Length; ++i)
                {
                    _hue_stream.AddEffect(effects[i]);
                }
            }
            catch (Exception)
            {
                Assume.That(false);
            }

            _hue_stream.UnlockMixer();
        }

        private void AssertColor(Func<String, Boolean> successCondition, Action<String, double>  after, params Light[] lights) {
            Assert.AreNotEqual(0, lights.Length, "Incoming lights collection is empty");

            foreach (var light in lights)
            {
                DateTime a = DateTime.Now;
                String actualColor = _bridgeWrapperHelper.GetLightRGBColor(light.AsLightID());
                double diff = 0;
                while (!successCondition(actualColor) && diff < TIMEOUT_MS) {
                    actualColor = _bridgeWrapperHelper.GetLightRGBColor(light.AsLightID());
                    ThreadWaitFor(10);
                    diff = (DateTime.Now - a).TotalMilliseconds;
                }
                after(actualColor, diff);
            }
        }

        private void AssertSameColor(String expectedColor, params Light[] lights)
        {
            Func<string, bool> successCondition = (actualColor) => actualColor == expectedColor;
            AssertColor(
                successCondition,
                (actualColor, timeout) => Assert.IsTrue(successCondition(actualColor), $"Colors are not equal, expected={expectedColor}, actual={actualColor}, timeout={timeout} ms"),
                lights
            );
        }

        private void AssertDifferentColor(String expectedColor, params Light[] lights)
        {
            Func<string, bool> successCondition = (actualColor) => actualColor != expectedColor;
            AssertColor(
                successCondition,
                (actualColor, timeout) => Assert.IsTrue(successCondition(actualColor), $"Colors are equal, expected={expectedColor}, actual={actualColor}, timeout={timeout} ms"),
                lights
            );
        }

        [Test]
        [Ignore("HSDK-2347")]
        public void SimpleOverallAreaEffect()
        {
            AreaEffect effect = new AreaEffect("simpleAreaEffect", 0);
            effect.AddArea(Area.All);
            Color effectColor = new Color(0.0, 1.0, 0.0);
            effect.SetFixedColor(effectColor);

            AddEffectToEngine(effect);
            effect.Enable();

            ThreadWaitFor(100);

            _allLights.ForEach(light => AssertDifferentColor(RGB_BLACK, light));
        }

        [Test]
        [Ignore("HSDK-2347")]
        public void TwoAreaEffect()
        {
            AreaEffect rearLeftPart = new AreaEffect("rearLeft", 1);
            rearLeftPart.AddArea(Area.BackLeft);
            Color rearLeftColor = new Color(0.0, 1.0, 0.0);
            rearLeftPart.SetFixedColor(rearLeftColor);

            AreaEffect frontRightPart = new AreaEffect("frontRight", 2);
            frontRightPart.AddArea(Area.FrontRight);
            Color frontRightColor = new Color(1.0, 1.0, 1.0);
            frontRightPart.SetFixedColor(frontRightColor);

            AddEffectToEngine(rearLeftPart);
            AddEffectToEngine(frontRightPart);
            rearLeftPart.Enable();
            frontRightPart.Enable();

            ThreadWaitFor(200);

            Assert.Multiple(() =>
            {
                AssertDifferentColor(RGB_BLACK, _rearLeftLight);
                AssertDifferentColor(RGB_BLACK, _frontRightLight);
                AssertSameColor(RGB_BLACK, _rearRightLight);
                AssertSameColor(RGB_BLACK, _frontLeftLight);
            });
        }

        [Test]
        [Ignore("HSDK-2347")]
        public void LightSourceEffect()
        {
            LightSourceEffect effect = new LightSourceEffect();
            effect.SetColorAnimation(new ConstantAnimation(0.0)
                    , new ConstantAnimation(1.0)
                    , new ConstantAnimation(0.0));
            effect.SetPositionAnimation(new ConstantAnimation(0), new ConstantAnimation(0));
            effect.SetRadius(new ConstantAnimation(1.5));

            AddEffectToEngine(effect);
            effect.Enable();

            ThreadWaitFor(100);

            _allLights.ForEach(light => AssertDifferentColor(RGB_BLACK, light));
        }

        [Test]
        [Ignore("HSDK-2347")]
        public void LightIteratorEffect()
        {
            int OFFSET_DIRATION = 1000;

            LightIteratorEffect effect = new LightIteratorEffect();
            effect.SetColorAnimation(new TweenAnimation(0, 0, OFFSET_DIRATION, TweenType.Linear)
                    , new TweenAnimation(1, 1, OFFSET_DIRATION, TweenType.Linear)
                    , new TweenAnimation(0, 0, OFFSET_DIRATION, TweenType.Linear));

            effect.SetOpacityAnimation(new TweenAnimation(1, 1, OFFSET_DIRATION, TweenType.Linear));
            effect.SetOffset(OFFSET_DIRATION);
            effect.SetOrder(IterationOrder.IterationOrderClockwise);
            effect.SetMode(IterationMode.IterationModeCycle);

            AddEffectToEngine(effect);
            effect.Enable();

            AssertDifferentColor(RGB_BLACK, _rearRightLight);
            AssertSameColor(RGB_BLACK, _frontRightLight, _frontLeftLight, _rearLeftLight);

            ThreadWaitFor(OFFSET_DIRATION);
            AssertDifferentColor(RGB_BLACK, _rearLeftLight);
            AssertSameColor(RGB_BLACK, _rearRightLight, _frontRightLight, _frontLeftLight);


            ThreadWaitFor(OFFSET_DIRATION);
            AssertDifferentColor(RGB_BLACK, _frontLeftLight);
            AssertSameColor(RGB_BLACK, _frontRightLight, _rearRightLight, _rearLeftLight);

            ThreadWaitFor(OFFSET_DIRATION);
            AssertDifferentColor(RGB_BLACK, _frontRightLight);
            AssertSameColor(RGB_BLACK, _frontLeftLight, _rearLeftLight, _rearRightLight);
        }

        [Test]
        [Ignore("HSDK-2347")]
        public void ManualEffect()
        {
            ManualEffect effect = new ManualEffect();

            effect.SetIdToColor(_frontLeftLight.GetLightId().ToString(), new Color(1.0, 0.0, 0.0));
            effect.SetIdToColor(_frontRightLight.GetLightId().ToString(), new Color(0.0, 1.0, 0.0));
            effect.SetIdToColor(_rearLeftLight.GetLightId().ToString(), new Color(0.0, 0.0, 1.0));
            effect.SetIdToColor(_rearRightLight.GetLightId().ToString(), new Color(1.0, 1.0, 0.0));

            AddEffectToEngine(effect);
            effect.Enable();

            ThreadWaitFor(100);

            _allLights.ForEach(light => AssertDifferentColor(RGB_BLACK, light));
        }
    }
}
