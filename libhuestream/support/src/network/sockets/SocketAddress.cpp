/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <string>

#include "support/network/sockets/SocketAddress.h"

using std::string;

namespace support {

    SocketAddress::SocketAddress(string ip, unsigned int port) : _ip(ip),
                                                                 _port(port) { }

    const string& SocketAddress::get_ip() const {
        return _ip;
    }

    void SocketAddress::set_ip(const string& ip) {
        _ip = string(ip);
    }

    unsigned int SocketAddress::get_port() const {
        return _port;
    }

    void SocketAddress::set_port(unsigned int port) {
        _port = port;
    }

    bool SocketAddress::is_ipv4() const {
        return true;
    }
    
    bool SocketAddress::is_ipv6() const {
        return false;
    }

    SocketAddressProtocol SocketAddress::get_protocol() const {
        return SOCKET_ADDRESS_IPV4;
    }

}  // namespace support

