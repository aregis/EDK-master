/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <memory>
#include <vector>

#include "support/network/_test/Network.h"
#include "support/network/NetworkInterface.h"

using std::shared_ptr;

namespace support {

    class NetworkDelegate {
    public:
        /**
         @see lib/network/Network.h
         */
        virtual const std::vector<NetworkInterface> get_network_interfaces() = 0;
        virtual bool is_wifi_connected() = 0;
        virtual ~NetworkDelegate() = default;
    };
    
    // Default
    class NetworkDelegateImpl : public NetworkDelegate {
    public:
        /**
         @see lib/network/Network.h
         */
        const std::vector<NetworkInterface> get_network_interfaces() override {
            // Get network interfaces from real network class
            return Network::get_network_interfaces();
        }
        bool is_wifi_connected() override {
            return Network::is_wifi_connected();
        }
    };
    
    class NetworkDelegator {
    public:
        /**
         @see lib/network/Network.h
         */
        static const std::vector<NetworkInterface> get_network_interfaces();
        static bool is_wifi_connected();
        
        /* delegate */
       
        /**
         Set the delegate
         @note   Initially NetworkDelegateImpl is set as delegate
         @return The previous delegate, nullptr if no delegate has been set
         */
        static shared_ptr<NetworkDelegate> set_delegate(shared_ptr<NetworkDelegate> delegate);
        
    private:
        /**
         Constructor declared private, so no instance of this class can be created
         */
        NetworkDelegator();
    };

}  // namespace support
