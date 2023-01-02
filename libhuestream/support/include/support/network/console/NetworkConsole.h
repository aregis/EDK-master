/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <string>
#include <vector>

#if defined(_DURANGO)
    const std::vector<NetworkInterface> Network::get_network_interfaces() {
        std::vector<NetworkInterface> network_interfaces;
        if (_default_network_interface_set) {
            network_interfaces.push_back(_default_network_interface);
        }
        return network_interfaces;
    }
#elif defined(_ORBIS)
#include <libnetctl.h>
    const std::vector<NetworkInterface> Network::get_network_interfaces() {
        std::vector<NetworkInterface> network_interfaces;

        if (_default_network_interface_set) {
            network_interfaces.push_back(_default_network_interface);
            return network_interfaces;
        }

        int netState;
        if (sceNetCtlGetState(&netState) == SCE_OK) {
            if (netState == SCE_NET_CTL_STATE_IPOBTAINED) {
                SceNetCtlInfo info;
                sceNetCtlGetInfo(SCE_NET_CTL_INFO_IP_ADDRESS, &info);
                auto ipAddress = string(info.ip_address);
                
                NetworkInterface network_interface;
                network_interface.set_up(true);
                network_interface.set_loopback(false);
                network_interface.set_ip(ipAddress);
                network_interface.set_inet_type(INET_IPV4);
                network_interface.set_name("");

                network_interfaces.push_back(network_interface);
            }
        }

        return network_interfaces;
    }
#elif defined(_NX)
#include <nn/nifm.h>
#include <nn/nifm/nifm_ApiIpAddress.h>
    const std::vector<NetworkInterface> Network::get_network_interfaces() {
        std::vector<NetworkInterface> network_interfaces;

        if (_default_network_interface_set) {
            network_interfaces.push_back(_default_network_interface);
            return network_interfaces;
        }

        if (nn::nifm::IsNetworkAvailable()) {
            in_addr ipAddress;
            in_addr subnetMask;
            in_addr defaultGateway;
            in_addr preferredDns;
            in_addr alternateDns;

            nn::Result result = nn::nifm::GetCurrentIpConfigInfo(&ipAddress, &subnetMask, &defaultGateway, &preferredDns, &alternateDns);
            if (result.IsSuccess()) {
                ipAddressStr = string(nn::socket::InetNtoa(ipAddress));
                
                NetworkInterface network_interface;
                network_interface.set_up(true);
                network_interface.set_loopback(false);
                network_interface.set_ip(ipAddressStr);
                network_interface.set_inet_type(INET_IPV4);
                network_interface.set_name("");

                network_interfaces.push_back(network_interface);
            }
        }

        return network_interfaces;
    }
#endif

