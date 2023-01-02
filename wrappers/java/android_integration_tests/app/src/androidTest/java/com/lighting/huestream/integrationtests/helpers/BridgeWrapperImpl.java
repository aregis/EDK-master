package com.lighting.huestream.integrationtests.helpers;

import org.json.JSONException;
import org.junit.Assert;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

import org.json.JSONArray;
import org.json.JSONObject;

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
    public String getLightRBGColor(ILightID lightId) {
        try {
            return getLightRBGColor_internal(lightId);
        } catch (JSONException e) {
            Assert.fail("JSONException popped up");
        }

        return null;
    }

    @Override
    public Integer getEntertainmentGroupId() {
        try {
            return getEntertainmentGroupId_internal();
        } catch (JSONException e) {
            Assert.fail("JSONException popped up");
        }

        return -1;
    }

    @Override
    public void removeEntertainmentGroups() {
        try {
            removeEntertainmentGroups_internal();
        } catch (JSONException e) {
            Assert.fail("JSONException popped up");
        }
    }

    @Override
    public List<ILightID> getLLCLightsIDs() {
        try {
            return getLLCLightsIDs_internal();
        } catch (JSONException e) {
            Assert.fail("JSONException popped up");
        }

        return null;
    }

    @Override
    public void includeLightsIntoGroup(List<ILightID> lights, Integer targetGroupId) {
        try {
            includeLightsIntoGroup_internal(lights, targetGroupId);
        } catch (JSONException e) {
            Assert.fail("JSONException popped up");
        }
    }

    @Override
    public void setLightsCoordinates(Integer groupId, List<ILightCoordinate> lightCoordinates) {
        try {
            setLightsCoordinates_internal(groupId, lightCoordinates);
        } catch (JSONException e) {
            Assert.fail("JSONException popped up");
        }
    }

    @Override
    public void cleanupUser() {
        try {
            JSONArray response = new JSONArray(Network.performDeleteRequest(buildApiUrl("config/whitelist/" + _userName)).body);
            getSuccessNode(response);
        } catch (JSONException e) {
            Assert.fail("JSONException popped up");
        }
    }

    private void removeEntertainmentGroups_internal() throws JSONException {
        List<Integer> groups = getAllEntertainmentGroups();
        if (!groups.isEmpty()) {

            for(Integer groupId : groups) {
                String url = buildApiUrl("groups/" + groupId.toString());

                JSONArray response = new JSONArray(Network.performDeleteRequest(url).body);
                getSuccessNode(response);
            }
        }
    }

    private String getLightRBGColor_internal(ILightID lightId) throws JSONException {
        final String stipUrl = "http://" + _ip4Address + ":" + _tcpPort + "/stip/devices/" + lightId.getMac();
        final JSONObject response = new JSONObject(Network.performGetRequest(stipUrl).body);
        Assert.assertNotNull("Stip Light status response is null", response);

        final JSONObject stateNode = (JSONObject) response.get("state");
        Assert.assertNotNull("state node is null", stateNode);

        final String rgbText = (String) stateNode.get("rgb");
        Assert.assertNotNull("RGB text value is null", rgbText);

        return rgbText;
    }

    private Integer getEntertainmentGroupId_internal() throws JSONException {
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


    private List<ILightID> getLLCLightsIDs_internal() throws JSONException {
        JSONObject lightsResponse = new JSONObject(Network.performGetRequest(buildApiUrl("lights")).body);
        List<ILightID> result = new ArrayList<>();

        Iterator<String> lightIdIterator = lightsResponse.keys();
        while (lightIdIterator.hasNext()) {
            final String lightId = lightIdIterator.next();

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


    private void includeLightsIntoGroup_internal(List<ILightID> lights, Integer targetGroupId) throws JSONException {
        JSONArray lightsArray = new JSONArray();
        for (ILightID id : lights) {
            lightsArray.put(id.getLightId().toString());
        }

        JSONObject request = new JSONObject();
        request.put("lights", lightsArray);

        Network.Response response = Network.performUpdateRequest(buildApiUrl("groups/" + targetGroupId), request, Network.UPDATE_REQUEST.PUT);
        getSuccessNode(new JSONArray(response));
    }


    private void setLightsCoordinates_internal(Integer groupId, List<ILightCoordinate> lightCoordinates) throws JSONException {
        JSONObject locationsNode = new JSONObject();

        for (ILightCoordinate coordinate: lightCoordinates) {
            JSONArray coordArray = new JSONArray();
            coordArray.put(coordinate.getX());
            coordArray.put(coordinate.getY());
            coordArray.put(0);

            locationsNode.put(String.valueOf(coordinate.getLightId()), (Object) coordArray);
        }
        JSONObject request = new JSONObject();
        request.put("locations", (Object) locationsNode);

        JSONArray response = new JSONArray(
                Network.performUpdateRequest(buildApiUrl("groups/" + groupId), request, Network.UPDATE_REQUEST.PUT).body);
        Assert.assertNotNull("Update locations response is null", response);

        for (int i = 0; i < response.length(); ++i) {
            JSONObject lightNode = response.getJSONObject(i);
            Assert.assertNotNull("Invalid response format", lightNode);

            JSONObject successNode = (JSONObject) lightNode.get("success");
            Assert.assertNotNull("Creating group was not successful", successNode);
        }
    }

    Integer createEntertainmentGroup() throws JSONException {
        JSONObject request = new JSONObject();
        request.put("type", "Entertainment");
        JSONArray lightsArray = new JSONArray();
        request.put("lights", (Object) lightsArray);
        request.put("class", "TV");

        Network.Response response = Network.performUpdateRequest(buildApiUrl("groups"), request, Network.UPDATE_REQUEST.POST);
        JSONObject successNode = (JSONObject) getSuccessNode(new JSONArray(response.body));

        String groupIDValue = (String)successNode.get("id");
        Assert.assertNotNull("Can not find group id", groupIDValue);

        return Integer.parseInt(groupIDValue);
    }

    List<Integer> getAllEntertainmentGroups() throws JSONException {
        JSONObject groupsResponse = new JSONObject(Network.performGetRequest(buildApiUrl("groups")).body);
        Assert.assertNotNull("Groups JSON object is NULL", groupsResponse);
        List<Integer> result = new ArrayList<>();

        Iterator<String> groupIdIterator = groupsResponse.keys();
        while (groupIdIterator.hasNext()) {
            final String groupId = groupIdIterator.next();
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

    Object getSuccessNode(final JSONArray response) throws JSONException {
        Assert.assertNotNull("Create/Update/Delete response is null", response);

        JSONObject responseRoot = (JSONObject) response.get(0);
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
