package com.philips.lighting.hue.sdk.wrapper.utilities;

import android.content.Context;
import android.net.ConnectivityManager;
import android.net.LinkProperties;
import android.net.Network;
import android.net.NetworkInfo;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;
import android.support.annotation.Nullable;
import android.util.Log;

import java.io.FileDescriptor;
import java.math.BigInteger;
import java.net.InetAddress;
import java.net.UnknownHostException;
import java.nio.ByteOrder;

public class WifiUtil {

    WifiManager wifiManager;
	ConnectivityManager connectivityManager;

    public WifiUtil(final Context context) {
        wifiManager = (WifiManager) context.getSystemService(Context.WIFI_SERVICE);
		connectivityManager = (ConnectivityManager)context.getSystemService(Context.CONNECTIVITY_SERVICE);
    }

    @SuppressWarnings("unused")
    public @Nullable
    Boolean isEnabled() {
        if (wifiManager != null) {
            return wifiManager.isWifiEnabled();
        } else {
            return null;
        }
    }

    @SuppressWarnings("unused")
    private @Nullable
    Boolean isWifiOnAndConnected() {
        if (wifiManager != null) {
            if (wifiManager.isWifiEnabled()) {

                WifiInfo wifiInfo = wifiManager.getConnectionInfo();

                if (wifiInfo.getNetworkId() == -1) {
                    return false;
                }
                return true;
            } else {
                return false;
            }
        } else {
            return null;
        }
    }

    /* Wifi network interface name (e.g. wlan0) can only be retrieved
       using NetworkInterface. Therefore we return the an empty identifier instead. */
    @SuppressWarnings("unused")
    private String getName() {
        return "";
    }

    @SuppressWarnings("unused")
    private @Nullable
    String getSSID() {
        if (wifiManager != null) {
            WifiInfo wifiInfo = wifiManager.getConnectionInfo();

            if (wifiInfo != null) {
                return wifiInfo.getSSID();
            }
            return null;
        }
        return null;
    }

    @SuppressWarnings("unused")
    private @Nullable
    String getIpV4Address() {
        if (wifiManager != null) {
            int ipAddress = wifiManager.getConnectionInfo().getIpAddress();

            // Convert little-endian to big-endianif needed
            if (ByteOrder.nativeOrder().equals(ByteOrder.LITTLE_ENDIAN)) {
                ipAddress = Integer.reverseBytes(ipAddress);
            }

            byte[] ipByteArray = BigInteger.valueOf(ipAddress).toByteArray();

            String ipAddressString;
            try {
                ipAddressString = InetAddress.getByAddress(ipByteArray).getHostAddress();
            } catch (UnknownHostException ex) {
                Log.e("WIFIIP", "Unable to get host address.");
                ipAddressString = null;
            }
            return ipAddressString;
        } else {
            return null;
        }
    }
	
	private void networkBindSocket(FileDescriptor fileDescriptor, String adapterName) {
        Network[] networks = connectivityManager.getAllNetworks();
        for (Network n : networks) {
            try {
                LinkProperties prop = connectivityManager.getLinkProperties(n);
                if (prop.getInterfaceName().equals(adapterName)) {
                    n.bindSocket(fileDescriptor);
                }
            } catch (java.io.IOException e) {
                Log.e("WIFIIP", "networkBindSocket: error binding socket to network ", e);
            } catch (java.lang.NullPointerException e) {
                Log.e("WIFIIP", "networkBindSocket: error getting network properties ", e);
            } catch (java.lang.NoSuchMethodError e) {
                Log.e("WIFIIP", "networkBindSocket: Android API >=23 should be used ", e);
            }
        }
    }
}