package com.philips.lighting.hue.sdk.wrapper.utilities;

import java.net.Inet4Address;
import java.net.Inet6Address;
import java.net.InetAddress;
import java.net.NetworkInterface;
import java.net.SocketException;
import java.util.ArrayList;
import java.util.Enumeration;
import java.util.List;

public class NetUtil {

    List<NetworkInterface> networkInterfaces = null;

    NetUtil() {
        networkInterfaces = new ArrayList<NetworkInterface>();
        try {
            Enumeration<NetworkInterface> interfaces = NetworkInterface.getNetworkInterfaces();
            while (interfaces.hasMoreElements()) {
                networkInterfaces.add(interfaces.nextElement());
            }
        } catch (SocketException se) {
            se.printStackTrace();
        }
    }

    @SuppressWarnings("unused")
    private int getInterfaceCount() {
        return networkInterfaces.size();
    }

    private NetworkInterface getInterface(int index) {
        return networkInterfaces.get(index);
    }

    @SuppressWarnings("unused")
    private byte[] getInterfaceName(int index) {
        return NativeTools.StringToBytes(getInterface(index).getDisplayName());
    }

    @SuppressWarnings("unused")
    private byte[] getInterfaceIPv4Address(int index) {
        Enumeration<InetAddress> addresses = getInterface(index).getInetAddresses();
        while (addresses.hasMoreElements()) {
            InetAddress address = addresses.nextElement();
            if (address instanceof Inet4Address) {
                String ip = address.toString();
                if (ip.charAt(0) == '/') {
                    ip = ip.substring(1); // strip forward slash, leave pure ip
                }
                return NativeTools.StringToBytes(ip);
            }
        }
        return null; // it is possible that this interface has no ipv4 address
    }

    @SuppressWarnings("unused")
    private byte[] getInterfaceIPv6Address(int index) {
        Enumeration<InetAddress> addresses = getInterface(index).getInetAddresses();
        while (addresses.hasMoreElements()) {
            InetAddress address = addresses.nextElement();
            if (address instanceof Inet6Address) {
                String ip = address.toString();
                if (ip.charAt(0) == '/') {
                    ip = ip.substring(1); // strip forward slash, leave pure ip
                }
                return NativeTools.StringToBytes(ip);
            }
        }
        return null; // it is possible that this interface has no ipv4 address
    }

    @SuppressWarnings("unused")
    private boolean getInterfaceUp(int index) {
        try {
            return getInterface(index).isUp();
        } catch (SocketException e) {
            e.printStackTrace();
            return false;
        }
    }

    @SuppressWarnings("unused")
    private boolean getInterfaceLoopback(int index) {
        try {
            return getInterface(index).isLoopback();
        } catch (SocketException e) {
            e.printStackTrace();
            return false;
        }
    }
}