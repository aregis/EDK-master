package com.lighting.huestream.tests.integration;

import com.lighting.huestream.*;
import com.lighting.huestream.tests.integration.helpers.BridgeWrapperBuilder;
import com.lighting.huestream.tests.integration.helpers.IBridgeWrapper;
import com.lighting.huestream.tests.integration.helpers.Network;
import com.lighting.huestream.tests.integration.helpers.Light;
import org.json.simple.JSONObject;
import org.junit.Assert;
import org.json.simple.JSONArray;

import java.io.*;
import java.text.ParseException;
import java.util.Arrays;
import java.util.InvalidPropertiesFormatException;
import java.util.List;
import java.util.ArrayList;
import java.util.Map;
import java.util.stream.Collectors;
import java.nio.file.*;
import java.util.logging.Logger;

public class BaseTest {


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

    protected HueStream createStream(StreamingMode streamingMode) {
        readBridgePropertiesIfNeeded();

        StreamSettings streamSettings = new StreamSettings();
        streamSettings.SetStreamingPort(Integer.parseInt(_udp_port));

        Config config = new Config("JavaIntegrationTests", "PC", new PersistenceEncryptionKey("encryption_key"));
        config.SetStreamSettings(streamSettings);
        config.SetStreamingMode(streamingMode);
        return new HueStream(config);
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
        return _allLights.stream()
                .map(it -> it.asLightCoordinate())
                .collect(Collectors.toList());
    }

    private void readBridgePropertiesIfNeeded() {
        if (_ipv4_address.isEmpty() || _bridge_id.isEmpty() || _tcp_port.isEmpty() || _udp_port.isEmpty() || _user.isEmpty() || _clientKey.isEmpty()) {
            final String FILE_NAME = "ANT_OPTS.cfg";

            try {
                final String fileContents = readFile(FILE_NAME);

                // attempting to parse and verify the file contents
                Map<String, String> params = Arrays.stream(fileContents.split(" "))
                        .filter(token -> !token.isEmpty())
                        .map(token -> token.replaceAll("[\"']", "")) // removing the quotes - for Windows
                        .map(token -> token.split("="))
                        .collect(Collectors.toMap(
                                pair -> pair[0], //key
                                pair -> pair[1]  // value
                        ));

                _udp_port = params.getOrDefault("-Dhue_streaming_port", DEFAULT_UDP_PORT);
                _bridge_id = params.getOrDefault("-Dhue_bridge_id", DEFAULT_BRIDGE_ID);
                _ssl_port = params.getOrDefault("-Dhue_https_port", DEFAULT_SSL_PORT);
                _tcp_port = params.getOrDefault("-Dhue_http_port", DEFAULT_TCP_PORT);
                _ipv4_address = params.getOrDefault("-Dhue_ip", DEFAULT_IP_ADDRESS);
                _user = params.getOrDefault("-Dhue_username", DEFAULT_USERNAME);
                _clientKey = params.getOrDefault("-Dhue_clientkey", DEFAULT_CLIENT_KEY);
            } catch (Exception e) {
                System.out.println("Exception, setting default params " + e);

                _ipv4_address = DEFAULT_IP_ADDRESS;
                _tcp_port = DEFAULT_TCP_PORT;
                _bridge_id = DEFAULT_BRIDGE_ID;
                _udp_port = DEFAULT_UDP_PORT;
                _ssl_port = DEFAULT_SSL_PORT;
                _user = DEFAULT_USERNAME;
                _clientKey = DEFAULT_CLIENT_KEY;
            }

            System.out.println(String.format("IP_address=%s, tcp_port=%s, udp_port=%s, ssl_port=%s, bridge_id=%s, username=%s, clientkey=%s",
                    _ipv4_address, _tcp_port, _udp_port, _ssl_port, _bridge_id, _user, _clientKey));

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

    protected void pushLink(Boolean enable) {
        final StringBuilder urlBuilder = new StringBuilder();
        urlBuilder.append("http://")
                .append(_ipv4_address)
                .append(":")
                .append(_tcp_port)
                .append("/stop/linkbutton");

        Network.Response response = Network.performUpdateRequest(urlBuilder.toString(), "", Network.UPDATE_REQUEST.POST);
        Assert.assertEquals(204, response.code);
    }

    protected void clearPersistenceData() {
        Path persistencePath = Paths.get(PERSISTENCE_LOCATION);
        try {
           Files.deleteIfExists(persistencePath);
        } catch (IOException Exception) {
            Logger.getGlobal().info("Could not find peristence file in " + PERSISTENCE_LOCATION);
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
    private String _ipv4_address = "";
    private String _bridge_id = "";
    private String _tcp_port = "";
    private String _udp_port = "";
    private String _ssl_port = "";


    private static String DEFAULT_IP_ADDRESS = "192.168.1.51";
    private static String DEFAULT_TCP_PORT = "60202";
    private static String DEFAULT_UDP_PORT = "60202";
    private static String DEFAULT_SSL_PORT = "61202";
    private static String DEFAULT_BRIDGE_ID = "001788fffe1ffd08";
    private static String PERSISTENCE_LOCATION = "bridge.json";
    private static String DEFAULT_USERNAME = "integrationTest";
    private static String DEFAULT_CLIENT_KEY = "integrationTest";
}
