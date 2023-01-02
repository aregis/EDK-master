package com.lighting.huestream.tests.integration;

import com.lighting.huestream.*;
import com.lighting.huestream.tests.integration.helpers.IBridgeWrapper;
import org.junit.*;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

public class ConnectivityTests extends BaseTest {
    @BeforeClass
    public static void loadNativeLibrary() {
        System.loadLibrary("huestream_java_native");
    }

    @Before
    public void setUp() {
        _bridge = initializeBridge();
        _bridgeWrapperHelper = initializeBridgeWrapper();
        initializeBridgeResources();

        resetStream();
    }

    @After
    public void tearDown() {
        _hue_stream.ShutDown();
        clearPersistenceData();
    }

    private void checkStreamConnectionValid() {
        Assert.assertEquals("Connection result must be Streaming", ConnectResult.Streaming, _hue_stream.GetConnectionResult());
        Assert.assertTrue("Bridge is not authorized", _hue_stream.GetActiveBridge().IsAuthorized());
        Assert.assertTrue("Bridge is not streaming", _hue_stream.IsBridgeStreaming());
    }

    private void checkStreamNotConnected() {
        Assert.assertEquals("Connection result must be Unitialized", ConnectResult.Uninitialized, _hue_stream.GetConnectionResult());
        Assert.assertFalse("Bridge is still authorized", _hue_stream.GetActiveBridge().IsAuthorized());
        Assert.assertFalse("Bridge is still streaming", _hue_stream.IsBridgeStreaming());
        Assert.assertFalse("Bridge is still connectable", _hue_stream.GetActiveBridge().IsConnectable());
    }

    private void resetStream() {
        if (_hue_stream != null && _hue_stream.IsBridgeStreaming())
        {
            _hue_stream.ShutDown();
        }

        _hue_stream = createStream(StreamingMode.STREAMING_MODE_UDP);
        _hue_stream.RegisterFeedbackHandler(new FeedBackHandler());
    }

    @Test
    public void connectManual_Valid() {
        _hue_stream.ConnectManualBridgeInfo(_bridge);
        checkStreamConnectionValid();
    }

    @Test
    public void connectManual_InvalidClientKey() {
        _bridge.SetClientKey("Invalid Client Key");
        _hue_stream.ConnectManualBridgeInfo(_bridge);

        Assert.assertEquals("Stream should expect action", ConnectResult.ActionRequired, _hue_stream.GetConnectionResult());
        Assert.assertFalse("Client key should not be valid", _hue_stream.GetActiveBridge().IsValidClientKey());
    }

    @Test
    public void connectManual_Valid_Async() {
        
        // Start listening on the ID_USERPROCEDURE_FINISHED message, connect to the bridge
        FeedbackMessageObserver messageObserver = createObserverForMessage(FeedbackMessage.Id.ID_USERPROCEDURE_FINISHED);
        _hue_stream.ConnectManualBridgeInfoAsync(_bridge);

        
        messageObserver.waitForMessage(DEFAULT_TIMEOUT_MS);
            

        checkStreamConnectionValid();
    }

    @Test
    public void connectManual_InvalidUsername_Async() {
        _bridge.SetUser("шnvalid_username");

        FeedbackMessageObserver messageObserver = createObserverForMessage(FeedbackMessage.Id.ID_FINISH_RETRIEVING_FAILED);
        _hue_stream.ConnectManualBridgeInfoAsync(_bridge);


        messageObserver.waitForMessage(DEFAULT_TIMEOUT_MS);
    }

    @Test
    public void connectManual_ResetStream_ConnectFromPersistence() {
        _hue_stream.ConnectManualBridgeInfo(_bridge);
        checkStreamConnectionValid();

        resetStream();
        checkStreamNotConnected();

        // Connect using persisted bridge info
        _hue_stream.ConnectBridge();
        checkStreamConnectionValid();
    }

    @Test
    public void connectManual_ResetStream_ConnectBackgroundFromPersistence()
    {
        _hue_stream.ConnectManualBridgeInfo(_bridge);
        checkStreamConnectionValid();

        // Stop streaming
        resetStream();
        checkStreamNotConnected();

        // Connect using persisted bridge info
        _hue_stream.ConnectBridgeBackground();
        checkStreamConnectionValid();
    }

    @Test
    public void connectManual_NoEntertainmentGroups_AddGroup_Connect() {
        _bridgeWrapperHelper.removeEntertainmentGroups();
        Assert.assertFalse( "There is still selected group on bridge", _hue_stream.GetActiveBridge().IsGroupSelected());
        _hue_stream.ConnectManualBridgeInfo(_bridge);
        Assert.assertEquals("Stream should expect action", ConnectResult.ActionRequired, _hue_stream.GetConnectionResult());

        // Stop streaming.
        resetStream();
        checkStreamNotConnected();

        Integer entertainmentGroup = _bridgeWrapperHelper.getEntertainmentGroupId();
        _bridge.SelectGroup(entertainmentGroup.toString());

        // Try to connect again
        _hue_stream.ConnectManualBridgeInfo(_bridge);
        checkStreamConnectionValid();
    }

    @Test
    public void setEncryptionKey_BridgePersistedAndRetrieved() {
        _hue_stream.GetConfig().GetAppSettings().SetStorageEncryptionKey("encryption_key");
        _hue_stream.ConnectManualBridgeInfo(_bridge);
        checkStreamConnectionValid();

        resetStream();

        _hue_stream.GetConfig().GetAppSettings().SetStorageEncryptionKey("encryption_key");
        _hue_stream.LoadBridgeInfo();
        Bridge loadedBridge = _hue_stream.GetActiveBridge();
        Assert.assertEquals("Loaded bridge id is not equal to initial one's", loadedBridge.GetId(), _bridge.GetId());
        Assert.assertEquals("Loaded bridge model id is not equal to initial one's", loadedBridge.GetModelId(), _bridge.GetModelId());
    }


    @Test
    public void retrieveBridgeWithWrongEncryptionKey_RetrievedBridgeIsEmpty() {
        _hue_stream.GetConfig().GetAppSettings().SetStorageEncryptionKey("right_encryption_key");
        _hue_stream.ConnectManualBridgeInfo(_bridge);
        checkStreamConnectionValid();

        resetStream();

        _hue_stream.GetConfig().GetAppSettings().SetStorageEncryptionKey("wrong_encryption_key");
        _hue_stream.LoadBridgeInfo();
        Bridge loadedBridge = _hue_stream.GetActiveBridge();
        Assert.assertTrue("Loaded bridge is not empty", loadedBridge.IsEmpty());
    }

    @Test
    public void bridgeGetsCertificatePinnedAndPersistedAfterSuccessfulConnection() {
        Assert.assertTrue(_bridge.GetCertificate().isEmpty());

        _hue_stream.ConnectManualBridgeInfo(_bridge);
        checkStreamConnectionValid();

        String bridgeCertificate = _bridge.GetCertificate(); 
        Assert.assertFalse(bridgeCertificate.isEmpty());
            
        resetStream();
        checkStreamNotConnected();
            
        // Connect to the same bridge again, it should have the same certificate
        _hue_stream.ConnectBridge();
        checkStreamConnectionValid();
        Assert.assertEquals(bridgeCertificate, _hue_stream.GetActiveBridge().GetCertificate());
            
        resetStream();
        checkStreamNotConnected();

        // Load bridge, it should have certificate loaded from persistence
        _hue_stream.LoadBridgeInfo();
        Assert.assertEquals(bridgeCertificate, _hue_stream.GetActiveBridge().GetCertificate());
    }

    @Test
    @Ignore("HSDK-2767")
    public void setIncorrectCertificateForBridge_ConnectionFails() {
        Assert.assertTrue(_bridge.GetCertificate().isEmpty());
        _bridge.SetCertificate("incorrect_certificate");

        FeedbackMessageObserver messageObserver = createObserverForMessage(FeedbackMessage.Id.ID_FINISH_RETRIEVING_FAILED);
        _hue_stream.ConnectManualBridgeInfoAsync(_bridge);

        
        messageObserver.waitForMessage(DEFAULT_TIMEOUT_MS);
    }

    @Test
    public void connectBridge_NoBridgesPersisted_DiscoveryStarted_BridgeFound()
    {
        clearPersistenceData();
        
        FeedbackMessageObserver messageObserver = createObserverForMessage(FeedbackMessage.Id.ID_START_SEARCHING);
        _hue_stream.ConnectBridgeAsync();
        messageObserver.waitForMessage(DEFAULT_TIMEOUT_MS);

        messageObserver = createObserverForMessage(FeedbackMessage.Id.ID_FINISH_SEARCH_BRIDGES_FOUND);
        messageObserver.waitForMessage(DISCOVERY_TIMEOUT_MS);
    }
    
    @Test
    public void connectBridgeBackground_NoBridgesPersisted_DiscoveryStarted()
    {
        clearPersistenceData();
        
        FeedbackMessageObserver messageObserver = createObserverForMessage(FeedbackMessage.Id.ID_START_SEARCHING);
        _hue_stream.ConnectBridgeBackgroundAsync();
        
        messageObserver.waitForMessage(DEFAULT_TIMEOUT_MS);
    }
    
    @Test
    public void connectManual_Success_ResetBridgeInfo_StreamStops_ConnectBridge_DiscoveryStarted()
    {
        _hue_stream.ConnectManualBridgeInfo(_bridge);
        checkStreamConnectionValid();
        
        _hue_stream.ResetBridgeInfo();
        Assert.assertFalse("Bridge is still streaming", _hue_stream.IsBridgeStreaming());
        
        FeedbackMessageObserver messageObserver = createObserverForMessage(FeedbackMessage.Id.ID_START_SEARCHING);
        _hue_stream.ConnectBridgeAsync();
        
        messageObserver.waitForMessage(DEFAULT_TIMEOUT_MS);
    }
    
    @Test
    public void connectBridge_NoBridgesPersisted_DiscoveryStarted_AbortConnection_StreamStopped()
    {
        clearPersistenceData();
        
        FeedbackMessageObserver messageObserver = createObserverForMessage(FeedbackMessage.Id.ID_START_SEARCHING);
        _hue_stream.ConnectBridgeAsync();
        messageObserver.waitForMessage(DEFAULT_TIMEOUT_MS);
        
        _hue_stream.AbortConnecting();
        checkStreamNotConnected();
    }

    @Test
    public void connectBridgeManualIp_StartsAuthorizing()
    {
        FeedbackMessageObserver messageObserver = createObserverForMessage(FeedbackMessage.Id.ID_PRESS_PUSH_LINK);            
        _hue_stream.ConnectBridgeManualIpAsync(_bridge.GetIpAddress() + ":" + _bridge.GetTcpPort());
        messageObserver.waitForMessage(DEFAULT_TIMEOUT_MS);
    }

    @Test
    public void connectBridgeManualIp_WrongIp_Failed()
    {
        FeedbackMessageObserver messageObserver = createObserverForMessage(FeedbackMessage.Id.ID_FINISH_RETRIEVING_FAILED);            
        _hue_stream.ConnectBridgeManualIpAsync("123.456.789.123");
        messageObserver.waitForMessage(DEFAULT_TIMEOUT_MS);
    }
}
