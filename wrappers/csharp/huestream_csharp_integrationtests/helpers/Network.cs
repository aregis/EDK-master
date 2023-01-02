using System;
using System.IO;
using System.Net;
using System.Text;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;

public static class Network
{
    public enum UPDATE_REQUEST
    {
        POST,
        PUT
    }

    public struct Response
    {
        public Response(HttpStatusCode code, JContainer body)
        {
            this.code = code;
            this.body = body;
        }

        public readonly HttpStatusCode code;
        public readonly JContainer body;
    }

    public static Response PerformUpdateRequest(String fullUrl, String serializedRequest, UPDATE_REQUEST type)
    {
        var url = new Uri(fullUrl);
        byte[] requestByteArray = Encoding.UTF8.GetBytes(serializedRequest);

        var httpRequest = WebRequest.CreateHttp(url);
        httpRequest.Method = type.ToString();
        httpRequest.ContentType = "application/x-www-form-urlencoded";
        httpRequest.Accept = "application/json";
        httpRequest.ContentLength = serializedRequest.Length;

        // Write data to stream
        using (var stream = httpRequest.GetRequestStream())
        {
            stream.Write(requestByteArray, 0, requestByteArray.Length);
            stream.Close();
        }

        // Get response
        using (var response = (HttpWebResponse)httpRequest.GetResponse())
        using (var stream = response.GetResponseStream())
        using (var reader = new StreamReader(stream))
        {
            var jsonResponse = reader.ReadToEnd();
            return new Response(response.StatusCode, JArray.Parse(jsonResponse));
        }
    }

    public static Response PerformUpdateRequest(String fullUrl, JObject request, UPDATE_REQUEST type)
    {
        return PerformUpdateRequest(fullUrl, request.ToString(Formatting.None), type);
    }

    public static Response PerformDeleteRequest(String fullUrl)
    {
        var url = new Uri(fullUrl);

        var httpRequest = WebRequest.CreateHttp(url);
        httpRequest.Method = "DELETE";
        httpRequest.Accept = "application/json";


        using (var response = (HttpWebResponse)httpRequest.GetResponse())
        using (var stream = response.GetResponseStream())
        using (var reader = new StreamReader(stream))
        {
            var jsonResponse = reader.ReadToEnd();
            return new Response(response.StatusCode, JArray.Parse(jsonResponse));
        }
    }

    public static Response PerformGetRequest(String fullUrl)
    {
        var url = new Uri(fullUrl);

        var httpRequest = WebRequest.CreateHttp(url);
        httpRequest.Method = WebRequestMethods.Http.Get;
        httpRequest.Accept = "application/json";

        using (var response = (HttpWebResponse)httpRequest.GetResponse())
        using (var stream = response.GetResponseStream())
        using (var reader = new StreamReader(stream))
        {
            var jsonResponse = reader.ReadToEnd();
            return new Response(response.StatusCode, JObject.Parse(jsonResponse));
        }
    }
}