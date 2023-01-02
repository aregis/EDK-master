package com.lighting.huestream.integrationtests;


import android.util.Log;
import android.os.Environment;

import com.lighting.huestream.Bridge;
import com.lighting.huestream.BridgeSettings;
import com.lighting.huestream.Config;
import com.lighting.huestream.HueStream;
import com.lighting.huestream.StreamSettings;
import com.lighting.huestream.StreamingMode;
import com.lighting.huestream.PersistenceEncryptionKey;
import com.lighting.huestream.FeedbackMessage;

import com.lighting.huestream.integrationtests.helpers.BridgeWrapperBuilder;
import com.lighting.huestream.integrationtests.helpers.IBridgeWrapper;
import com.lighting.huestream.integrationtests.helpers.Network;
import com.lighting.huestream.integrationtests.helpers.Light;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;
import org.junit.Assert;

import java.io.BufferedReader;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.List;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.InvalidPropertiesFormatException;
import java.util.Map;
import java.io.File;

public class BaseTest {
    private static final String TAG = BaseTest.class.getName();

    protected static final int DEFAULT_TIMEOUT_MS = 3000;
    protected static final int DISCOVERY_TIMEOUT_MS = 120000;
    protected static final int LIGHTS_COUNT = 4;

    protected Bridge _bridge = null;
    protected HueStream _hue_stream = null;
    protected IBridgeWrapper _bridgeWrapperHelper = null;
    protected Light _frontLeftLight = null;
    protected Light _frontRightLight = null;
    protected Light _rearLeftLight = null;
    protected Light _rearRightLight = null;
    protected List<Light> _allLights = null;

    protected Bridge initializeBridge() {
        readBridgePropertiesIfNeeded();

        Bridge result = new Bridge(_bridge_id, _ipv4_address, true, new BridgeSettings());

        result.SetUser(_user);
        result.SetClientKey(_clientKey);
        result.SetTcpPort(_tcp_port);
        result.SetSslPort(_ssl_port);
        result.EnableSsl();

        return result;
    }

    protected IBridgeWrapper initializeBridgeWrapper()
    {
        readBridgePropertiesIfNeeded();

        BridgeWrapperBuilder builder = new BridgeWrapperBuilder();
        builder.withUserName(_user)
                .withClientKey(_clientKey)
                .withIPv4Address(_ipv4_address)
                .withTcpPort(_tcp_port)
                .withBridgeId(_bridge_id);

        return builder.build();
    }

    protected void initializeBridgeResources() {
        Integer entertainmentGroupId = _bridgeWrapperHelper.getEntertainmentGroupId();
        List<IBridgeWrapper.ILightID> lights = _bridgeWrapperHelper.getLLCLightsIDs();

        Assert.assertTrue(lights.size() >= LIGHTS_COUNT);
        if (lights.size() > LIGHTS_COUNT) {
            lights = lights.subList(0, LIGHTS_COUNT);
        }

        initializeLights(lights, entertainmentGroupId);
        _bridge.SelectGroup(entertainmentGroupId.toString());
    }

    private void initializeLights(List<IBridgeWrapper.ILightID> lights, int entertainmentGroupId) {
        Assert.assertEquals("Amount of lights is not equal to LIGHTS_COUNT", LIGHTS_COUNT, lights.size());

        _frontLeftLight = new Light(Light.Position.FrontLeft, lights.get(0));
        _frontRightLight = new Light(Light.Position.FrontRight, lights.get(1));
        _rearLeftLight = new Light(Light.Position.RearLeft, lights.get(2));
        _rearRightLight = new Light(Light.Position.RearRight, lights.get(3));

        _allLights = new ArrayList<>();
        _allLights.add(_frontRightLight);
        _allLights.add(_frontLeftLight);
        _allLights.add(_rearRightLight);
        _allLights.add(_rearLeftLight);


        _bridgeWrapperHelper.includeLightsIntoGroup(lights, entertainmentGroupId);
        _bridgeWrapperHelper.setLightsCoordinates(entertainmentGroupId, lightsAsLightCoordinates());
    }

    private List<IBridgeWrapper.ILightCoordinate> lightsAsLightCoordinates() {
        List<IBridgeWrapper.ILightCoordinate> result = new ArrayList<>();
        for (Light ligth: _allLights) {
            result.add(ligth.asLightCoordinate());
        }

        return result;
    }

    protected HueStream createStream(StreamingMode streamingMode) {
        readBridgePropertiesIfNeeded();

        StreamSettings streamSettings = new StreamSettings();
        streamSettings.SetStreamingPort(Integer.parseInt(_udp_port));

        Config config = new Config("JavaIntegrationTests", "PC", new PersistenceEncryptionKey("encryption_key"));
        config.SetStreamSettings(streamSettings);
        config.SetStreamingMode(streamingMode);
        return new HueStream(config);
    }

    protected void threadWaitFor(int milliseconds) {
        try {
            Thread.sleep(milliseconds);
        } catch (InterruptedException e) {
            Assert.fail("Thread was interrupted when sleeping");
        }
    }

    private void readBridgePropertiesIfNeeded() {
        if (_ipv4_address.isEmpty() || _bridge_id.isEmpty() || _tcp_port.isEmpty() || _udp_port.isEmpty()) {
            final String FILE_NAME = "sdcard/GRADLE_OPTS.cfg";

            try {
                final String fileContents = readFile(FILE_NAME);

                String[] paramsArray = fileContents.split(" ");
                Map<String, String> params = new HashMap<>();
                for (String paramString : paramsArray) {
                    if (!paramString.isEmpty()) {
                        paramString = paramString.trim();
                        String[] paramsPair = paramString.split("=");

                        Assert.assertEquals(paramsPair.length, 2);
                        params.put(paramsPair[0], paramsPair[1]);
                    }
                }

                _udp_port = params.get("-Dhue_streaming_port");
                if (_udp_port == null) {
                    _udp_port = DEFAULT_UDP_PORT;
                }

                _ssl_port = params.get("-Dhue_https_port");
                if (_ssl_port == null) {
                    _ssl_port = DEFAULT_SSL_PORT;
                }

                _tcp_port = params.get("-Dhue_http_port");
                if (_tcp_port == null) {
                    _tcp_port = DEFAULT_TCP_PORT;
                }

                _bridge_id = params.get("-Dhue_bridge_id");
                if (_bridge_id == null) {
                    _bridge_id = DEFAULT_BRIDGE_ID;
                }

                _ipv4_address = params.get("-Dhue_ip");
                if (_ipv4_address == null) {
                    _ipv4_address = DEFAULT_IP_ADDRESS;
                }

                _user = params.get("-Dhue_username");
                if (_user == null) {
                    _user = DEFAULT_USERNAME;
                }

                _clientKey = params.get("-Dhue_clientkey");
                if (_clientKey == null) {
                    _clientKey = DEFAULT_CLIENT_KEY;
                }
            } catch (Exception e) {
                System.out.println("Exception, setting default params " + e);

                _ipv4_address = DEFAULT_IP_ADDRESS;
                _tcp_port = DEFAULT_TCP_PORT;
                _bridge_id = DEFAULT_BRIDGE_ID;
                _udp_port = DEFAULT_UDP_PORT;
            }

            System.out.println(String.format("IP_address=%s, tcp_port=%s, udp_port=%s, ssl_port=%s, bridge_id=%s",
                    _ipv4_address, _tcp_port, _udp_port, _ssl_port, _bridge_id));

        }
    }

    private String readFile(final String fileName) throws IOException {
        InputStream fis = new FileInputStream(fileName);
        InputStreamReader isr = new InputStreamReader(fis);
        BufferedReader reader = new BufferedReader(isr);

        StringBuilder fileContents = new StringBuilder();
        String line;
        while ((line = reader.readLine()) != null) {
            fileContents.append(line);
        }

        return fileContents.toString();
    }

    protected void cleanupUser() {
        if (_user.isEmpty()) {
            return;
        }

        _user = "";
        _clientKey = "";
    }

    protected void pushLink() {
        final StringBuilder urlBuilder = new StringBuilder();
        urlBuilder.append("http://")
                .append(_ipv4_address)
                .append(":")
                .append(_tcp_port)
                .append("/stip/linkbutton");

        Network.Response response = Network.performUpdateRequest(urlBuilder.toString(), "", Network.UPDATE_REQUEST.POST);
        Assert.assertEquals(204, response.code);
    }

    protected void clearPersistenceData() {
        File peristenceFile = new File(PERSISTENCE_LOCATION);

        if(peristenceFile.exists()) {
            peristenceFile.delete();
        }
        
    }

    public FeedbackMessageObserver createObserverForMessage(FeedbackMessage.Id message) {
        FeedbackMessageObserver messageObserver = new FeedbackMessageObserver();
        _hue_stream.RegisterFeedbackHandler(messageObserver);

        messageObserver.StartListening(message);
        return messageObserver;
    }


    private String _user = "";
    private String _clientKey = "";
    private static String _ipv4_address = "";
    private static String _bridge_id = "";
    private static String _tcp_port = "";
    private static String _udp_port = "";
    private static String _ssl_port = "";

    private static final String PERSISTENCE_LOCATION = "bridge.json";
    private static final String DEFAULT_IP_ADDRESS = "192.168.1.51";
    private static final String DEFAULT_TCP_PORT = "60202";
    private static final String DEFAULT_UDP_PORT = "60202";
    private static final String DEFAULT_SSL_PORT = "61202";
    private static final String DEFAULT_BRIDGE_ID = "001788fffe1ffd08";
    private static final String DEFAULT_USERNAME = "integrationTest";
    private static final String DEFAULT_CLIENT_KEY = "integrationTest";
}
