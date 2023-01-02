package com.lighting.huestream.tests.integration.helpers;

public class BridgeWrapperBuilder {
    public BridgeWrapperBuilder() { }

    /**
     * Constructs the Bridge Wrapper. If some params are not passed, returns null
     * @return IBridgeWrapper instance, if all parameters have been provided; null otherwise.
     */
    public IBridgeWrapper build() {
        if (_userName.isEmpty()
                || _clientKey.isEmpty()
                || _ip4Address.isEmpty()
                || _bridgeId.isEmpty()
                || _tcpPort.isEmpty()) {
            return null;
        }

        return new BridgeWrapperImpl(_userName, _clientKey, _ip4Address, _bridgeId, _tcpPort);
    }

    public BridgeWrapperBuilder withUserName(final String userName) {
        _userName = userName;
        return this;
    }

    public BridgeWrapperBuilder withClientKey(final String clientKey) {
        _clientKey = clientKey;
        return this;
    }

    public BridgeWrapperBuilder withIPv4Address(final String address) {
        _ip4Address = address;
        return this;
    }

    public BridgeWrapperBuilder withBridgeId(final String bridgeId) {
        _bridgeId = bridgeId;
        return this;
    }

    public BridgeWrapperBuilder withTcpPort(final String port) {
        _tcpPort = port;
        return this;
    }

    private String _userName = "";
    private String _clientKey = "";
    private String _ip4Address = "";
    private String _bridgeId = "";
    private String _tcpPort = "";
}
