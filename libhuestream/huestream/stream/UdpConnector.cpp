/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <huestream/stream/UdpConnector.h>

#include <iostream>
#include <memory>

namespace huestream {

    UdpConnector::UdpConnector() :
            _socketUdp(nullptr),
            _peerAddress("", 0),
            _connectionState(ConnectionState::Disconnected) {
    }

    UdpConnector::~UdpConnector() {
        Disconnect();
    }

    bool UdpConnector::Connect(BridgePtr bridge, unsigned short port) {
        if (_connectionState == ConnectionState::Connected) {
            Disconnect();
        }

        _socketUdp = std::make_shared<support::SocketUdp>(support::SocketAddress("0.0.0.0", 0));

        _peerAddress.set_ip(RemovePortFromIp(bridge->GetIpAddress()));
        _peerAddress.set_port(port);

        if (_socketUdp->bind() != support::SOCKET_STATUS_OK)
            return false;
        if (_socketUdp->connect_sync(_peerAddress) != support::SOCKET_STATUS_OK)
            return false;

        _connectionState = ConnectionState::Connected;
        return true;
    }

    bool UdpConnector::Send(const char *buffer, unsigned int bufferSize) {
        if (_connectionState != ConnectionState::Connected)
            return false;

        return _socketUdp->sync_send((unsigned char *) buffer, bufferSize) != support::SOCKET_STATUS_OK;
    }

    void UdpConnector::Disconnect() {
        if (_connectionState == ConnectionState::Connected) {
            _socketUdp->close();
            _connectionState = ConnectionState::Disconnected;
        }
    }

    std::string UdpConnector::RemovePortFromIp(std::string ip) const {
        std::istringstream iss(ip);
        std::string ipWithoutPort;
        std::getline(iss, ipWithoutPort, ':');
        return ipWithoutPort;
    }

}  // namespace huestream
