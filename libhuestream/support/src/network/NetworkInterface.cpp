/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <regex>
#include <string>

#include <boost/range/algorithm/partition.hpp>

#include "support/network/NetworkInterface.h"

using std::regex;
using std::regex_search;
using std::smatch;
using std::string;
using std::string;

namespace support {


    NetworkInterface::NetworkInterface() : NetworkInterface("", NetworkInetType::INET_IPV4, "", false, false, NetworkAdapterType::NETWORK_ADAPTER_TYPE_UNKNOWN, false) {}
    NetworkInterface::NetworkInterface(string ip, NetworkInetType inet_type) : NetworkInterface(std::move(ip), inet_type, "", false, false, NetworkAdapterType::NETWORK_ADAPTER_TYPE_UNKNOWN, false) {}

    NetworkInterface::NetworkInterface(string ip,
                                       NetworkInetType inet_type,
                                       string name,
                                       bool up,
                                       bool loopback,
                                       NetworkAdapterType adapter_type,
                                       bool is_connected) :
        _name(std::move(name)),
        _ip(std::move(ip)),
        _ipv4_num(ip_to_uint(_ip)),
        _netmask_num(0),
        _inet_type(inet_type),
        _up(up),
        _loopback(loopback),
        _adapter_type(adapter_type),
        _is_connected(is_connected) {}

    const string& NetworkInterface::get_name() const {
        return _name;
    }

    void NetworkInterface::set_name(const string& name) {
        _name = string(name);
    }

    const string& NetworkInterface::get_ip() const {
        return _ip;
    }

    void NetworkInterface::set_ip(const string& ip) {
        _ip = string(ip);
        if (ip.length() <= 15) {
            _ipv4_num = ip_to_uint(ip);
        }
    }

    void NetworkInterface::set_netmask(const string& netmask) {
        _netmask_num = ip_to_uint(netmask);
    }

    NetworkInetType NetworkInterface::get_inet_type() const {
        return _inet_type;
    }
    
    void NetworkInterface::set_inet_type(NetworkInetType inet_type) {
        _inet_type = inet_type;
    }

    bool NetworkInterface::is_up() const {
        return _up;
    }

    void NetworkInterface::set_up(bool up) {
        _up = up;
    }

    bool NetworkInterface::is_loopback() const {
        return _loopback;
    }

    void NetworkInterface::set_loopback(bool loopback) {
        _loopback = loopback;
    }

    bool NetworkInterface::is_private() const {
        if (_inet_type != INET_IPV4) {
            // Only IPV4 supported for now
            return false;
        }

        static std::regex ip_regex{"^([0-9]{1,3})\\.([0-9]{1,3})\\.([0-9]{1,3})\\.([0-9]{1,3})$"};
        smatch result;
        // Split the ip into pieces
        if (!regex_search(_ip, result, ip_regex)) {
            return false;
        }

        // Get all bytes from the ip address
        int b1 = atoi(string(result[1].first, result[1].second).c_str());
        int b2 = atoi(string(result[2].first, result[2].second).c_str());

        // 10.x.x.x
        // 127.0.0.1
        if (b1 == 10
            || b1 == 127) {
            return true;
        }
        // 172.16.0.0 - 172.31.255.255
        if ((b1 == 172)
            && (b2 >= 16)
            && (b2 <= 31)) {
            return true;
        }
        // 192.168.0.0 - 192.168.255.255
        if ((b1 == 192) && (b2 == 168))
            return true;

        return false;
    }

    uint32_t NetworkInterface::ip_to_uint(const string& ip) {
        int a, b, c, d;
        uint32_t addr = 0;

        if (sscanf(ip.c_str(), "%d.%d.%d.%d", &a, &b, &c, &d) != 4)
            return 0;

        addr = a << 24;
        addr |= b << 16;
        addr |= c << 8;
        addr |= d;

        return addr;
    }

    bool NetworkInterface::is_in_subnet(uint32_t ip) {
        if (_inet_type != INET_IPV4 && _netmask_num == 0 && _ipv4_num != 0) {
            // Only IPV4 supported for now
            return false;
        }

        if (ip == 0) {
            return false;
        }

        uint32_t net_lower = (_ipv4_num & _netmask_num);
        uint32_t net_upper = (net_lower | (~_netmask_num));
        return (ip >= net_lower && ip <= net_upper);
    }

    NetworkAdapterType NetworkInterface::get_adapter_type() const {
        return _adapter_type;
    }

    void NetworkInterface::set_adapter_type(NetworkAdapterType adapter_type) {
        _adapter_type = adapter_type;
    }

    void NetworkInterface::set_is_connected(bool is_connected) {
        _is_connected = is_connected;
    }

    bool NetworkInterface::get_is_connected() const {
        return _is_connected;
    }

    std::vector<NetworkInterface> prioritize(
            std::vector<NetworkInterface> network_interfaces) {
        boost::range::partition(network_interfaces, [](NetworkInterface network_interface) {
            return network_interface.get_adapter_type() == NetworkAdapterType::NETWORK_ADAPTER_TYPE_WIRELESS
                   && network_interface.get_is_connected();
        });

        return network_interfaces;
    }

}  // namespace support
