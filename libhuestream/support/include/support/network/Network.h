/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <string>
#include <vector>

#include "support/network/NetworkInterface.h"
#include "support/util/VectorOperations.h"

namespace support {

    class Network {
    public:
        /**
         Get all available network interfaces
         @return List of the found network interfaces
         */
        static const std::vector<NetworkInterface> get_network_interfaces();
        static void set_default_network_interface(std::string ip, std::string name, NetworkInetType type);

        static bool is_wifi_connected();

        /*
         Get the name of the interface that's connected to the private network with the provided subnet
         @private_ip ip address in numeric form
         */
        static std::string get_local_interface_name(uint32_t private_ip);

        /*
         Tell system to use provided interface by default
         NB: currently only needed on Android, which refuses to use the local interface if it has
             no Internet. We will force the system to use it (for the current process)
         @param interface_name name of the interface to set (e.g. "wlan0")
         */
        static void set_system_default_interface(std::string interface_name);

        static void set_interface_for_socket(int file_descriptor, const std::string& interface_name);

    private:
        /**
         Constructor declared private, so no instance of this class can be created
         */
        Network();

        static NetworkAdapterType get_adapter_type(const NetworkInterface& network_interface);
        static bool is_network_interface_connected(const NetworkInterface& network_interface);

        static bool _default_network_interface_set;
        static NetworkInterface _default_network_interface;
    };

}  // namespace support

