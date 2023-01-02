package com.lighting.huestream.tests.integration.helpers;

import java.util.ArrayList;
import java.util.List;

import org.json.simple.JSONArray;
import org.json.simple.JSONObject;
import org.junit.Assert;

class BridgeWrapperImpl implements IBridgeWrapper {
    public BridgeWrapperImpl(final String userName, final String clientKey, final String ip4Address,
                             final String bridgeId, final String tcpPort) {
        _userName = userName;
        _clientKey = clientKey;
        _ip4Address = ip4Address;
        _bridgeId = bridgeId;
        _tcpPort = tcpPort;
    }

    @Override
    public void removeEntertainmentGroups() {
        List<Integer> groups = getAllEntertainmentGroups();
        if (!groups.isEmpty()) {

            for(Integer groupId : groups) {
                String url = buildApiUrl("groups/" + groupId.toString());

                JSONArray response = (JSONArray) Network.performDeleteRequest(url).body;
                getSuccessNode(response);
            }
        }
    }

    @Override
    public String getLightRBGColor(ILightID lightId) {
        final String stipUrl = "http://" + _ip4Address + ":" + _tcpPort + "/stip/devices/" + lightId.getMac();
        final JSONObject response = (JSONObject) Network.performGetRequest(stipUrl).body;
        Assert.assertNotNull("Stip Light status response is null", response);

        final JSONObject stateNode = (JSONObject) response.get("state");
        Assert.assertNotNull("state node is null", stateNode);

        final String rgbText = (String) stateNode.get("rgb");
        Assert.assertNotNull("RGB text value is null", rgbText);

        return rgbText;
    }

    @Override
    public Integer getEntertainmentGroupId() {
        List<Integer> groupIds = getAllEntertainmentGroups();

        if (groupIds.isEmpty()) {
            return createEntertainmentGroup();
        } else {
            return groupIds.get(0);
        }
    }

    class LightIDImpl implements ILightID {
        public LightIDImpl(Integer id, final String mac) {
            _id = id;
            _mac = mac;
        }

        @Override
        public Integer getLightId() {
            return _id;
        }

        @Override
        public String getMac() {
            return _mac;
        }

        final Integer _id;
        final String _mac;
    }

    @Override
    public List<ILightID> getLLCLightsIDs() {
        JSONObject lightsResponse = (JSONObject) Network.performGetRequest(buildApiUrl("lights")).body;
        List<ILightID> result = new ArrayList<>();

        for (Object key: lightsResponse.keySet()) {
            String lightId = (String) key;

            JSONObject lightInfo = (JSONObject) lightsResponse.get(lightId);
            String modelId = (String) lightInfo.get("modelid");
            String uniqueId = (String) lightInfo.get("uniqueid");
            uniqueId = uniqueId.substring(0, 23);

            if (modelId.contains("LLC")) {
                result.add(new LightIDImpl(Integer.parseInt(lightId), uniqueId));
            }
        }

        return result;
    }

    @Override
    public void includeLightsIntoGroup(List<ILightID> lights, Integer targetGroupId) {
        JSONArray lightsArray = new JSONArray();
        for (ILightID id : lights) {
            lightsArray.add(id.getLightId().toString());
        }

        JSONObject request = new JSONObject();
        request.put("lights", lightsArray);

        JSONArray response = (JSONArray)Network.performUpdateRequest(buildApiUrl("groups/" + targetGroupId), request, Network.UPDATE_REQUEST.PUT).body;
        getSuccessNode(response);
    }

    @Override
    public void setLightsCoordinates(Integer groupId, List<ILightCoordinate> lightCoordinates) {
        JSONObject locationsNode = new JSONObject();

        for (ILightCoordinate coordinate: lightCoordinates) {
            JSONArray coordArray = new JSONArray();
            coordArray.add(coordinate.getX());
            coordArray.add(coordinate.getY());
            coordArray.add(0);

            locationsNode.put(String.valueOf(coordinate.getLightId()), coordArray);
        }
        JSONObject request = new JSONObject();
        request.put("locations", locationsNode);

        JSONArray response = (JSONArray) Network.performUpdateRequest(buildApiUrl("groups/" + groupId), request, Network.UPDATE_REQUEST.PUT).body;
        Assert.assertNotNull("Update locations response is null", response);

        for (Object node : response) {
            JSONObject lightNode = (JSONObject) node;
            Assert.assertNotNull("Invalid response format", lightNode);

            JSONObject successNode = (JSONObject) lightNode.get("success");
            Assert.assertNotNull("Creating group was not successful", successNode);
        }
    }

    @Override
    public void cleanupUser() {
        if (_userName.isEmpty()) {
            return;
        }

        JSONArray response = (JSONArray)Network.performDeleteRequest(buildApiUrl("config/whitelist/" + _userName)).body;
        getSuccessNode(response);
    }

    Integer createEntertainmentGroup() {
        JSONObject request = new JSONObject();
        request.put("type", "Entertainment");
        JSONArray lightsArray = new JSONArray();
        request.put("lights", lightsArray);
        request.put("class", "TV");

        JSONArray response = (JSONArray)Network.performUpdateRequest(buildApiUrl("groups"), request, Network.UPDATE_REQUEST.POST).body;
        JSONObject successNode = (JSONObject) getSuccessNode(response);

        String groupIDValue = (String)successNode.get("id");
        Assert.assertNotNull("Can not find group id", groupIDValue);

        return Integer.parseInt(groupIDValue);
    }

    List<Integer> getAllEntertainmentGroups() {
        JSONObject groupsResponse = (JSONObject) Network.performGetRequest(buildApiUrl("groups")).body;
        Assert.assertNotNull("Groups JSON object is NULL", groupsResponse);
        List<Integer> result = new ArrayList<>();

        for (Object key: groupsResponse.keySet()) {
            final String groupId = (String) key;
            JSONObject group = (JSONObject) groupsResponse.get(groupId);

            String groupType = (String) group.get("type");
            if (groupType.equals("Entertainment")) {
                result.add(Integer.parseInt(groupId));
            }
        }

        return result;
    }

    String buildApiUrl(final String urlTail) {
        StringBuilder urlBuilder = new StringBuilder();
        urlBuilder.append("http://")
                .append(_ip4Address)
                .append(":")
                .append(_tcpPort)
                .append("/api/")
                .append(_userName)
                .append("/")
                .append(urlTail);

        return urlBuilder.toString();
    }

    Object getSuccessNode(final JSONArray response) {
        Assert.assertNotNull("Create/Update/Delete response is null", response);

        JSONObject responseRoot = (JSONObject) response.iterator().next();
        Assert.assertNotNull("Wrong structure of incoming response", responseRoot);

        Object successNode = responseRoot.get("success");
        Assert.assertNotNull("No \"success\" node is found", successNode);

        return successNode;
    }

    private final String _userName;
    private final String _clientKey;
    private final String _ip4Address;
    private final String _bridgeId;
    private final String _tcpPort;
}
