using System;
using System.IO;
using huestream;

namespace HueStreamExampleWPF
{
    /// <summary>
    /// Class that implements the IFeedbackMessageHandler from SWIG to handle libhuestream callbacks
    /// </summary>
    class Hue : IFeedbackMessageHandler
    {
        private readonly Config _config;
        private HueStream _hueStream;

        public Hue(string appName, string deviceName, string encryptionKey)
        {
            _config = new Config(appName, deviceName, new PersistenceEncryptionKey(encryptionKey));
        }

        ~Hue()
        {
            _hueStream?.ShutDown();
        }

        public void InitializeSimulator()
        {
            _config.SetStreamingMode(StreamingMode.STREAMING_MODE_UDP);
            _hueStream = new HueStream(_config);
            _hueStream.RegisterFeedbackHandler(this);
        }

        public void Initialize()
        {
            _config.SetStreamingMode(StreamingMode.STREAMING_MODE_DTLS);
            _hueStream = new HueStream(_config);
            _hueStream.RegisterFeedbackHandler(this);
        }

        /// <summary>
        /// FeedbackMessage is an event transferred from IFeedbackMessageHandler 
        /// </summary>
        public event EventHandler<FeedbackMessage> ConnectMessage;

        public override void NewFeedbackMessage(FeedbackMessage message)
        {
            ConnectMessage?.Invoke(this, message);
        }

        public void ConnectCurrent()
        {
            _hueStream.ConnectBridgeAsync();
        }

        public void ConnectManualIp(string ip)
        {
            _hueStream.ConnectBridgeManualIpAsync(ip);
        }

        public void PlayExplosion()
        {
            var effect = new ExplosionEffect("boem", 1);
            var color = new Color();
            color.SetR(1);
            color.SetG(1);
            color.SetB(1);
            color.SetAlpha(1);
            var location = new Location(0, 0);
            effect.PrepareEffect(color, location, 2000, 2, 500, 500);
            _hueStream.AddEffect(effect);
            effect.Enable();
        }

        public void PlayBrightnessSawEffect()
        {
            var effect = new BrightnessSawEffect("zaag", 1);
            effect.PrepareEffect();
            _hueStream.AddEffect(effect);
            effect.Enable();
        }

        public void ShutDown()
        {
            _hueStream?.ShutDown();
            _hueStream = null;
        }

        public bool IsBridgeStreaming()
        {
            return _hueStream?.IsBridgeStreaming() ?? false;
        }
    }
}
