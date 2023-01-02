using System.Collections.Generic;
using NUnit.Framework;
using huestream;
using System.Linq;
using System;
using System.IO;
using System.Net;
using System.Text;
using System.Threading;
using Newtonsoft.Json.Linq;

namespace huestream_tests
{
    public class TestBase
    {
        protected const String APPLICATION_NAME = "CSharpIntegrationTests";
        protected const String PERSISTENCE_LOCATION = "bridge.json";

        private String _ipv4_address = "192.168.1.51";
        private String _bridge_id = "001788fffe1ffd08";
        private String _tcp_port = "60202";
        private String _udp_port = "60202";
        private String _ssl_port = "61202";
        private String _username = "integrationTest";
        private String _client_key = "integrationTest";
        
        protected const int CONNECTION_TIMEOUT_MS = 3000;
        protected const int DISCOVERY_TIMEOUT_MS = 120000;
        protected const int LIGHTS_COUNT = 4;

        protected FeedbackMessageHandler _message_handler;
        protected HueStream _hue_stream;
        protected Bridge _bridge;
        protected IBridgeWrapper _bridgeWrapperHelper;

        protected Light _frontLeftLight;
        protected Light _frontRightLight;
        protected Light _rearLeftLight;
        protected Light _rearRightLight;
        protected List<Light> _allLights;

        public TestBase()
        {
            if (TestContext.Parameters.Count > 0)
            {
                _ipv4_address = TestContext.Parameters["hue_ip"];
                _bridge_id = TestContext.Parameters["hue_bridge_id"];
                _udp_port = TestContext.Parameters["hue_streaming_port"];
                _ssl_port = TestContext.Parameters["hue_https_port"];
                _tcp_port = TestContext.Parameters["hue_http_port"];
                _username = TestContext.Parameters["hue_username"];
                _client_key = TestContext.Parameters["hue_clientkey"];
            }
        }

        public Bridge InitializeBridge()
        {
            Bridge result = new Bridge(_bridge_id, _ipv4_address, true, new BridgeSettings());

            result.SetUser(_username);
            result.SetClientKey(_client_key);
            result.SetTcpPort(_tcp_port);
            result.SetSslPort(_ssl_port);
            result.EnableSsl();
            
            return result;
        }

        public IBridgeWrapper InitializeBridgeWrapper()
        {
            BridgeWrapperBuilder builder = new BridgeWrapperBuilder();
            builder.WithUserName(_username)
                   .WithClientKey(_client_key)
                   .WithIPv4Address(_ipv4_address)
                   .WithTcpPort(_tcp_port)
                   .WithBridgeId(_bridge_id);

            return builder.Build();
        }
        
        protected void InitializeBridgeResources()
        {
            int entertainmentGroupId = _bridgeWrapperHelper.GetEntertainmentGroupId();
            List<ILightID> lights = _bridgeWrapperHelper.GetLLCLightsIDs();

            Assert.IsTrue(lights.Count >= LIGHTS_COUNT);
            if (lights.Count > LIGHTS_COUNT)
            {
                lights = lights.GetRange(0, LIGHTS_COUNT);
            }

            InitializeLights(lights, entertainmentGroupId);
            _bridge.SelectGroup(entertainmentGroupId.ToString());
        }

        private void InitializeLights(List<ILightID> lights, int entertainmentGroupId)
        {
            Assert.AreEqual(LIGHTS_COUNT, lights.Count, "Amount of lights is not equal to LIGHTS_COUNT");

            _frontLeftLight = new Light(Light.Position.FrontLeft, lights[0]);
            _frontRightLight = new Light(Light.Position.FrontRight, lights[1]);
            _rearLeftLight = new Light(Light.Position.RearLeft, lights[2]);
            _rearRightLight = new Light(Light.Position.RearRight, lights[3]);

            _allLights = new List<Light>
            {
                _frontRightLight,
                _frontLeftLight,
                _rearRightLight,
                _rearLeftLight
            };
            _bridgeWrapperHelper.IncludeLightsIntoGroup(lights, entertainmentGroupId);
            _bridgeWrapperHelper.SetLightsCoordinates(entertainmentGroupId, LightsAsLightCoordinates());
        }

        private List<ILightCoordinate> LightsAsLightCoordinates()
        {
            return _allLights.Select(x => x.AsLightCoordinate()).ToList();
        }

        protected HueStream CreateStream(StreamingMode streamingMode)
        {
            StreamSettings streamSettings = new StreamSettings();
            streamSettings.SetStreamingPort(int.Parse(_udp_port));

            Config config = new Config(APPLICATION_NAME, Environment.OSVersion.Platform.ToString(), new PersistenceEncryptionKey("encryption_key"));
            config.SetStreamSettings(streamSettings);
            config.SetStreamingMode(streamingMode);
            return new HueStream(config);
        }

        protected void CleanupUser()
        {
            if (_username == "")
            {
                return;
            }
            
            _username = "";
            _client_key = "";
        }

        protected void PushLink()
        {
            StringBuilder urlBuilder = new StringBuilder();
            urlBuilder.Append("http://")
                    .Append(_ipv4_address)
                    .Append(":")
                    .Append(_tcp_port)
                    .Append("/stip/linkbutton");

            var response = Network.PerformUpdateRequest(urlBuilder.ToString(), "", Network.UPDATE_REQUEST.POST);
            Assert.AreEqual(HttpStatusCode.NoContent, response.code, "Push link response is null");
        }

        protected void ClearPersistenceData()
        {
            if (File.Exists(PERSISTENCE_LOCATION))
            {
                File.Delete(PERSISTENCE_LOCATION);
            }
        }

        protected WaitHandle GetWaitHandleForMessage(FeedbackMessage.Id message)
        {
            var messageObserver = new FeedbackMessageObserver();

            var waitHandle = new AutoResetEvent(false);
            messageObserver.SetEventWaitHandle(waitHandle);
            _hue_stream.RegisterFeedbackHandler(messageObserver);

            messageObserver.StartListening(message);

            return waitHandle;
        }
    }
}
