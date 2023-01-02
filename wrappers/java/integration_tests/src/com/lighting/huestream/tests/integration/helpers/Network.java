package com.lighting.huestream.tests.integration.helpers;

import org.json.simple.JSONArray;
import org.json.simple.JSONObject;
import org.json.simple.parser.JSONParser;
import org.json.simple.parser.ParseException;

import java.io.*;
import java.net.HttpURLConnection;
import java.net.MalformedURLException;
import java.net.URL;
import java.util.logging.Logger;

public class Network {
    static private JSONParser parser = new JSONParser();

    public enum UPDATE_REQUEST {
        POST,
        PUT
    }

    public static class Response {
        public Response(Object body, int code) {
            this.body = body;
            this.code = code;
        }

        public final Object body;
        public final int code;
    }

    public static Response performUpdateRequest(final String fullUrl, final String serializedRequest, final UPDATE_REQUEST type) {
        try {
            URL url = new URL(fullUrl);
            HttpURLConnection httpConnection = (HttpURLConnection) url.openConnection();
            httpConnection.setRequestMethod(type.name());
            httpConnection.setRequestProperty("Content-Length", String.valueOf(serializedRequest.length()));
            httpConnection.setDoOutput(true);
            httpConnection.connect();

            BufferedWriter writer = new BufferedWriter(new OutputStreamWriter(httpConnection.getOutputStream()));
            writer.write(serializedRequest);
            writer.flush();
            writer.close();
        
            Object responseBody = null;
            try {
                responseBody = parser.parse(extractFromStream(httpConnection.getInputStream()));
            } catch (ParseException ignored) {}

            return new Response(responseBody, httpConnection.getResponseCode());
        } catch (MalformedURLException e) {
            Logger.getGlobal().warning("Malformed url, when doing the request: " + e);
        } catch (IOException e) {
            Logger.getGlobal().warning("IOException when opening url connection: " + e);
        }

        return null;
    }

    public static Response performUpdateRequest(final String fullUrl, final JSONObject request, final UPDATE_REQUEST type) {
        return performUpdateRequest(fullUrl, request.toString(), type);
    }

    public static Response performDeleteRequest(final String fullUrl) {
        try {
            URL url = new URL(fullUrl);
            HttpURLConnection httpConnection = (HttpURLConnection) url.openConnection();
            httpConnection.setRequestMethod("DELETE");

            Object responseBody = null;
            try {
                responseBody = parser.parse(extractFromStream(httpConnection.getInputStream()));
            } catch (ParseException ignored) {}

            return new Response(responseBody, httpConnection.getResponseCode());
        } catch (MalformedURLException e) {
            Logger.getGlobal().warning("Malformed url, when doing the request: " + e);
        } catch (IOException e) {
            Logger.getGlobal().warning("IOException when opening url connection: " + e);
        }

        return null;
    }

    public static Response performGetRequest(final String fullUrl) {
        try {
            URL url = new URL(fullUrl);
            HttpURLConnection httpConnection = (HttpURLConnection) url.openConnection();
            httpConnection.setRequestMethod("GET");

            Object responseBody = null;
            try {
                responseBody = parser.parse(extractFromStream(httpConnection.getInputStream()));
            } catch (ParseException ignored) {}

            return new Response(responseBody, httpConnection.getResponseCode());
        } catch (MalformedURLException e) {
            Logger.getGlobal().warning("Malformed url, when doing the request: " + e);
        } catch (IOException e) {
            Logger.getGlobal().warning("IOException when opening url connection: " + e);
        }

        return null;
    }

    private static String extractFromStream(final InputStream stream) throws IOException {
        final BufferedReader reader = new BufferedReader(new InputStreamReader(stream));
        final StringBuilder builder = new StringBuilder();

        String line;
        while ((line = reader.readLine()) != null) {
            builder.append(line);
        }

        return builder.toString();
    }
}
