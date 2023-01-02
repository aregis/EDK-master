using NUnit.Framework;
using huestream;
using System.Threading;

namespace huestream_tests
{
    [TestFixture()]
    public class TestBridgeConnectivity : TestBase
    {
        [SetUp]
        public void SetUp()
        {
            _bridge = InitializeBridge();
            _bridgeWrapperHelper = InitializeBridgeWrapper();

            InitializeBridgeResources();
            
            ResetStream();
        }

        [TearDown]
        public void TearDown()
        {
            _hue_stream.ShutDown();
            
            ClearPersistenceData();
        }

        private static void CheckStreamConnectionValid(HueStream hueStream)
        {
            Assert.AreEqual(hueStream.GetConnectionResult(), ConnectResult.Streaming, "Connection result must be Streaming");
            Assert.IsTrue(hueStream.GetActiveBridge().IsAuthorized(), "Bridge is not authorized");
            Assert.IsTrue(hueStream.IsBridgeStreaming(), "Bridge is not streaming");
        }

        private static void CheckStreamNotConnected(HueStream hueStream)
        {
            Assert.AreEqual(hueStream.GetConnectionResult(), ConnectResult.Uninitialized, "Connection result must be Unitialized");
            Assert.IsFalse(hueStream.GetActiveBridge().IsAuthorized(), "Bridge is still authorized");
            Assert.IsFalse(hueStream.IsBridgeStreaming(), "Bridge is still streaming");
            Assert.IsFalse(hueStream.GetActiveBridge().IsConnectable(), "Bridge is still connectable");
        }

        private void ResetStream()
        {
            if (_hue_stream != null && _hue_stream.IsBridgeStreaming())
            {
                _hue_stream.ShutDown();
            }

            _message_handler = new FeedbackMessageHandler();

            _hue_stream = CreateStream(StreamingMode.STREAMING_MODE_UDP);
            _hue_stream.RegisterFeedbackHandler(_message_handler);
        }

        [Test]
        public void ConnectManual_Valid()
        {
            _hue_stream.ConnectManualBridgeInfo(_bridge);
            CheckStreamConnectionValid(_hue_stream);
        }

        [Test]
        public void ConnectManual_InvalidClientKey()
        {
            _bridge.SetClientKey("Invalid Client Key");
            _hue_stream.ConnectManualBridgeInfo(_bridge);
            Assert.AreEqual(_hue_stream.GetConnectionResult(), ConnectResult.ActionRequired, "Stream should expect action");
            Assert.IsFalse(_hue_stream.GetActiveBridge().IsValidClientKey(), "Client key should not be valid");
        }

        [Test]
        public void ConnectManual_Valid_Async()
        {
            var waitHandle = GetWaitHandleForMessage(FeedbackMessage.Id.ID_USERPROCEDURE_FINISHED);
            
            _hue_stream.ConnectManualBridgeInfoAsync(_bridge);

            // Wait for the message
            Assert.IsTrue(waitHandle.WaitOne(CONNECTION_TIMEOUT_MS), "Bridge connection timed out");
            CheckStreamConnectionValid(_hue_stream);
        }

        [Test]
        public void ConnectManual_InvalidUsername_Async()
        {
            _bridge.SetUser("invalid_username");

            // Create message observer, connect event handle to it and attach it to the stream
            var messageObserver = new FeedbackMessageObserver();
            _message_handler = messageObserver;

            var waitHandle = new AutoResetEvent(false);
            messageObserver.SetEventWaitHandle(waitHandle);
            _hue_stream.RegisterFeedbackHandler(messageObserver);

            // Start listening on the ID_FINISH_RETRIEVING_FAILED message, connect to the bridge
            messageObserver.StartListening(FeedbackMessage.Id.ID_FINISH_RETRIEVING_FAILED);
            _hue_stream.ConnectManualBridgeInfoAsync(_bridge);

            // Wait for the message
            Assert.IsTrue(waitHandle.WaitOne(CONNECTION_TIMEOUT_MS), "Config retrieval did not fail");
        }

        [Test]
        public void ConnectManual_ResetStream_ConnectFromPersistence()
        {
            _hue_stream.ConnectManualBridgeInfo(_bridge);
            CheckStreamConnectionValid(_hue_stream);

            // Stop streaming
            ResetStream();
            CheckStreamNotConnected(_hue_stream);

            // Connect using persisted bridge info
            _hue_stream.ConnectBridge();
            CheckStreamConnectionValid(_hue_stream);
        }
        
        [Test]
        public void ConnectManual_ResetStream_ConnectBackgroundFromPersistence()
        {
            _hue_stream.ConnectManualBridgeInfo(_bridge);
            CheckStreamConnectionValid(_hue_stream);

            // Stop streaming
            ResetStream();
            CheckStreamNotConnected(_hue_stream);

            // Connect using persisted bridge info
            _hue_stream.ConnectBridgeBackground();
            CheckStreamConnectionValid(_hue_stream);
        }

        [Test]
        public void ConnectManual_NoEntertainmentGroups_AddGroup_Connect()
        {
            // Remove all entertainment groups on bridge and try to connect
            _bridgeWrapperHelper.RemoveEntertainmentGroups();
            Assert.IsFalse(_hue_stream.GetActiveBridge().IsGroupSelected(), "There is still selected group on bridge");
            _hue_stream.ConnectManualBridgeInfo(_bridge);
            Assert.AreEqual(_hue_stream.GetConnectionResult(), ConnectResult.ActionRequired, "Stream should expect action");

            // Stop streaming.
            ResetStream();
            CheckStreamNotConnected(_hue_stream);

            // Add new entertainment group and select it
            int entertainmentGroup = _bridgeWrapperHelper.GetEntertainmentGroupId();
            _bridge.SelectGroup(entertainmentGroup.ToString());

            // Try to connect again
            _hue_stream.ConnectManualBridgeInfo(_bridge);
            CheckStreamConnectionValid(_hue_stream);
        }
        
        
        [Test]
        public void SetEncryptionKey_BridgePersistedAndRetrieved()
        {
            _hue_stream.GetConfig().GetAppSettings().SetStorageEncryptionKey("encryption_key");
            
            _hue_stream.ConnectManualBridgeInfo(_bridge);
            CheckStreamConnectionValid(_hue_stream);

            ResetStream();
            
            // Load the bridge and compare it with the initial one
            _hue_stream.GetConfig().GetAppSettings().SetStorageEncryptionKey("encryption_key");
            _hue_stream.LoadBridgeInfo();
            Bridge loadedBridge = _hue_stream.GetActiveBridge();
            Assert.AreEqual(loadedBridge.GetId(), _bridge.GetId());
            Assert.AreEqual(loadedBridge.GetModelId(), _bridge.GetModelId());
        }
        
        [Test]
        public void RetrieveBridgeWithWrongEncryptionKey_RetrievedBridgeIsEmpty()
        {
            _hue_stream.GetConfig().GetAppSettings().SetStorageEncryptionKey("right_encryption_key");
            
            _hue_stream.ConnectManualBridgeInfo(_bridge);
            CheckStreamConnectionValid(_hue_stream);

            ResetStream();
            
            // Set a different encryption key, load the bridge
            _hue_stream.GetConfig().GetAppSettings().SetStorageEncryptionKey("wrong_encryption_key");
            _hue_stream.LoadBridgeInfo();
            Bridge loadedBridge = _hue_stream.GetActiveBridge();
            Assert.True(loadedBridge.IsEmpty());
        }

        [Test]
        public void BridgeGetsCertificatePinnedAndPersistedAfterSuccessfulConnection()
        {
            Assert.IsEmpty(_bridge.GetCertificate());
            
            _hue_stream.ConnectManualBridgeInfo(_bridge);
            CheckStreamConnectionValid(_hue_stream);

            string bridgeCertificate = _bridge.GetCertificate(); 
            Assert.IsNotEmpty(bridgeCertificate);
            
            ResetStream();
            CheckStreamNotConnected(_hue_stream);
            
            // Connect to the same bridge again, it should have the same certificate
            _hue_stream.ConnectBridge();
            CheckStreamConnectionValid(_hue_stream);
            Assert.AreEqual(bridgeCertificate, _hue_stream.GetActiveBridge().GetCertificate());
            
            ResetStream();
            CheckStreamNotConnected(_hue_stream);

            // Load bridge, it should have certificate loaded from persistence
            _hue_stream.LoadBridgeInfo();
            Assert.AreEqual(bridgeCertificate, _hue_stream.GetActiveBridge().GetCertificate());
        }

        [Test]
        [Ignore("HSDK-2767")]
        public void SetIncorrectCertificateForBridge_ConnectionFails()
        {
            Assert.IsEmpty(_bridge.GetCertificate());
            _bridge.SetCertificate("incorrect_certificate");

            WaitHandle waitHandle = GetWaitHandleForMessage(FeedbackMessage.Id.ID_FINISH_RETRIEVING_FAILED);
            _hue_stream.ConnectManualBridgeInfoAsync(_bridge);

            // Wait for the message
            Assert.IsTrue(waitHandle.WaitOne(CONNECTION_TIMEOUT_MS), "Config retrieval did not fail");
        }
        
        [Test]
        public void ConnectBridge_NoBridgesPersisted_DiscoveryStarted_BridgeFound()
        {
            ClearPersistenceData();
            
            WaitHandle waitHandle = GetWaitHandleForMessage(FeedbackMessage.Id.ID_START_SEARCHING);
            _hue_stream.ConnectBridgeAsync();
            Assert.IsTrue(waitHandle.WaitOne(CONNECTION_TIMEOUT_MS), "Bridge search did not start");
            
            waitHandle = GetWaitHandleForMessage(FeedbackMessage.Id.ID_FINISH_SEARCH_BRIDGES_FOUND);
            Assert.IsTrue(waitHandle.WaitOne(DISCOVERY_TIMEOUT_MS), "No bridges were found in the network");
        }
        
        [Test]
        public void ConnectBridgeBackground_NoBridgesPersisted_DiscoveryStarted()
        {
            ClearPersistenceData();
            
            WaitHandle waitHandle = GetWaitHandleForMessage(FeedbackMessage.Id.ID_START_SEARCHING);
            _hue_stream.ConnectBridgeBackgroundAsync();
            
            Assert.IsTrue(waitHandle.WaitOne(CONNECTION_TIMEOUT_MS), "Bridge search did not start");
        
            _hue_stream.AbortConnecting();
        }
        
        [Test]
        public void ConnectManual_Success_ResetBridgeInfo_StreamStops_ConnectBridge_DiscoveryStarted()
        {
            _hue_stream.ConnectManualBridgeInfo(_bridge);
            CheckStreamConnectionValid(_hue_stream);
            
            _hue_stream.ResetBridgeInfo();
            Assert.IsFalse(_hue_stream.IsBridgeStreaming(), "Bridge is still streaming");
            
            WaitHandle waitHandle = GetWaitHandleForMessage(FeedbackMessage.Id.ID_START_SEARCHING);
            _hue_stream.ConnectBridgeAsync();
            
            Assert.IsTrue(waitHandle.WaitOne(CONNECTION_TIMEOUT_MS), "Bridge search did not start");
        }
        
        [Test]
        public void ConnectBridge_NoBridgesPersisted_DiscoveryStarted_AbortConnection_StreamStopped()
        {
            ClearPersistenceData();
            
            WaitHandle waitHandle = GetWaitHandleForMessage(FeedbackMessage.Id.ID_START_SEARCHING);
            _hue_stream.ConnectBridgeAsync();
            Assert.IsTrue(waitHandle.WaitOne(CONNECTION_TIMEOUT_MS), "Bridge search did not start");
            
            _hue_stream.AbortConnecting();
            CheckStreamNotConnected(_hue_stream);
        }

        [Test]
        public void ConnectBridgeManualIp_StartsAuthorizing()
        {
            WaitHandle waitHandle = GetWaitHandleForMessage(FeedbackMessage.Id.ID_PRESS_PUSH_LINK);            
            _hue_stream.ConnectBridgeManualIpAsync(_bridge.GetIpAddress() + ":" + _bridge.GetTcpPort());
            Assert.IsTrue(waitHandle.WaitOne(CONNECTION_TIMEOUT_MS), "Push link start message was not received");
        }

        [Test]
        public void ConnectBridgeManualIp_WrongIp_Failed()
        {
            WaitHandle waitHandle = GetWaitHandleForMessage(FeedbackMessage.Id.ID_FINISH_RETRIEVING_FAILED);            
            _hue_stream.ConnectBridgeManualIpAsync("123.456.789.123");
            Assert.IsTrue(waitHandle.WaitOne(CONNECTION_TIMEOUT_MS), "Config retrieval did not fail");
        }
    }
}
