using System;
using System.Text;
using System.Collections.Generic;
using System.Net;
using System.IO;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using NUnit.Framework;

namespace huestream_tests
{
    class LightIDImpl : ILightID
    {
        public LightIDImpl(int id, String mac)
        {
            _id = id;
            _mac = mac;
        }

        public int GetLightId()
        {
            return _id;
        }

        public String GetMac()
        {
            return _mac;
        }

        int _id;
        String _mac;
    }

    class BridgeWrapperImpl : IBridgeWrapper
    {
        public BridgeWrapperImpl(String userName, String clientKey, String ip4Address, String bridgeId, String tcpPort)
        {
            _userName = userName;
            _ip4Address = ip4Address;
            _clientKey = clientKey;
            _bridgeId = bridgeId;
            _tcpPort = tcpPort;
        }

        public void RemoveEntertainmentGroups()
        {
            List<int> groups = GetAllEntertainmentGroups();

            if (groups.Count == 0)
                return;

            foreach (int groupId in groups)
            {
                String url = BuildApiUrl("groups/" + groupId);
                var response = Network.PerformDeleteRequest(url);
                GetSuccessNode((JArray) response.body);
            }
        }

        public int GetEntertainmentGroupId()
        {
            List<int> groupIds = GetAllEntertainmentGroups();

            if (groupIds.Count == 0)
            {
                return CreateEntertainmentGroup();
            }
            else
            {
                return groupIds[0];
            }
        }

        public String GetLightRGBColor(ILightID lightId)
        {
            String stipUrl = "http://" + _ip4Address + ":" + _tcpPort + "/stip/devices/" + lightId.GetMac();
            var response = (JObject) Network.PerformGetRequest(stipUrl).body;

            Assert.IsNotNull(response, "Stip Light status response is null");

            JObject stateNode = (JObject)response["state"];
            Assert.IsNotNull(stateNode, "state node is null");

            String rgbText = (String)stateNode["rgb"];
            Assert.IsNotNull("RGB text value is null", rgbText);

            return rgbText;
        }

        public List<ILightID> GetLLCLightsIDs()
        {
            var lightsResponse = (JObject) Network.PerformGetRequest(BuildApiUrl("lights")).body;
            var result = new List<ILightID>();

            foreach (var keyValue in lightsResponse)
            {
                String lightId = keyValue.Key;
                var lightInfo = (JObject)keyValue.Value;

                String modelId = (String)lightInfo["modelid"];
                String uniqueId = (String)lightInfo["uniqueid"];
                uniqueId = uniqueId.Substring(0, 23);

                if (modelId.Contains("LLC"))
                {
                    result.Add(new LightIDImpl(int.Parse(lightId), uniqueId));
                }
            }

            return result;
        }

        public void IncludeLightsIntoGroup(List<ILightID> lights, int targetGroupId)
        {
            JArray lightsArray = new JArray();
            foreach (ILightID id in lights)
            {
                lightsArray.Add(id.GetLightId().ToString());
            }

            JObject request = new JObject
            {
                ["lights"] = lightsArray
            };

            var response = Network.PerformUpdateRequest(BuildApiUrl("groups/" + targetGroupId), request, Network.UPDATE_REQUEST.PUT);
            GetSuccessNode((JArray) response.body);
        }

        public void SetLightsCoordinates(int groupId, List<ILightCoordinate> lightCoordinates)
        {
            var locationsNode = new JObject();

            foreach (ILightCoordinate coordinate in lightCoordinates)
            {
                JArray coordArray = new JArray();
                coordArray.Add(coordinate.GetX());
                coordArray.Add(coordinate.GetY());
                coordArray.Add(0);

                locationsNode[coordinate.GetLightId().ToString()] = coordArray;
            }

            var request = new JObject
            {
                ["locations"] = locationsNode
            };
            var response = Network.PerformUpdateRequest(BuildApiUrl("groups/" + groupId), request, Network.UPDATE_REQUEST.PUT);
            Assert.NotNull(response.body, "Update locations response is null");

            foreach (var node in (JArray) response.body)
            {
                var lightNode = (JObject)node;
                Assert.NotNull(lightNode, "Invalid response format");

                var successNode = (JObject)lightNode["success"];
                Assert.NotNull(successNode, "Creating group was not successful");
            }
        }

        public void CleanUpUser()
        {
            var response = Network.PerformDeleteRequest(BuildApiUrl("config/whitelist/" + _userName));
            GetSuccessNode((JArray) response.body);
        }

        int CreateEntertainmentGroup()
        {
            var lightsArray = new JArray{};

            var request = new JObject
            {
                ["type"] = "Entertainment",
                ["lights"] = lightsArray,
                ["class"] = "TV"
            };

            var response = Network.PerformUpdateRequest(BuildApiUrl("groups"), request, Network.UPDATE_REQUEST.POST);
            JObject successNode = (JObject)GetSuccessNode((JArray) response.body);

            var groupIDValue = (String)successNode["id"];
            Assert.NotNull(groupIDValue, "Can not find group id");

            return int.Parse(groupIDValue);
        }

        List<int> GetAllEntertainmentGroups()
        {
            JObject groupsResponse = (JObject)Network.PerformGetRequest(BuildApiUrl("groups")).body;
            Assert.NotNull(groupsResponse, "Groups JSON object is NULL");

            var result = new List<int>();

            foreach (var keyValue in groupsResponse)
            {
                String groupId = keyValue.Key;
                var group = (JObject)keyValue.Value;

                var groupType = (String)group["type"];
                if (groupType == "Entertainment")
                {
                    result.Add(int.Parse(groupId));
                }
            }

            return result;
        }

        String BuildApiUrl(String urlTail)
        {
            StringBuilder urlBuilder = new StringBuilder();
            urlBuilder.Append("http://")
                      .Append(_ip4Address)
                      .Append(":")
                      .Append(_tcpPort)
                      .Append("/api/")
                      .Append(_userName)
                      .Append("/")
                      .Append(urlTail);

            return urlBuilder.ToString();
        }

        Object GetSuccessNode(JArray response)
        {
            Assert.NotNull(response, "Create/Update/Delete group response is null");

            JToken responseRoot = response[0];
            Assert.NotNull(responseRoot, "Wrong structure of incoming response");

            var successNode = responseRoot["success"];
            Assert.NotNull(successNode, "Creating group was not successful");

            return successNode;
        }

        private readonly String _userName;
        private readonly String _clientKey;
        private readonly String _ip4Address;
        private readonly String _bridgeId;
        private readonly String _tcpPort;
    }
}