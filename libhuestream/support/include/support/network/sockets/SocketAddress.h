/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <string>

#include "support/util/StringUtil.h"

using std::string;

namespace support {
    
    enum SocketAddressProtocol {
        SOCKET_ADDRESS_IPV4,
        SOCKET_ADDRESS_IPV6
    };


    class SocketAddress {
    public:
        /**
         Construct with data
         @param ip   The ip address of the socket address
                     e.g. 192.168.1.1 (IPV4),
                          2607:f0d0:1002:51::4 (IPV6)
         @param port The port of the socket address
         */
        SocketAddress(string ip, unsigned int port);
        
        /** 
         Get ip
         @return The ip of the socket address
         */
        const string& get_ip() const;
        
        /**
         Set ip
         @param ip The ip of the socket address
         */
        void set_ip(const string& ip);
        
        /** 
         Get port
         @return The port of the socket address
         */
        unsigned int get_port() const;
        
        /**
         Set port
         @param port The port of the socket address
         */
        void set_port(unsigned int port);
        
        /** 
         Whether the ip address is an IPV4 address
         @return true when IPV4, false otherwise
         */
        bool is_ipv4() const;
        
        /** 
         Whether the ip address is an IPV6 address
         @return true when IPV6, false otherwise
         */
        bool is_ipv6() const;
        
        /** 
         Get the protocol of the ip address
         @return The protocol, which represents one the following values:
                 - SOCKET_ADDRESS_IPV4: IPV4 address
                 - SOCKET_ADDRESS_IPV6: IPV6 address
         */
        SocketAddressProtocol get_protocol() const;
        
    protected:
        /** the ip address
            format: 
            - IPV4: 192.168.1.1
            - IPV6: 2607:f0d0:1002:51::4 */
        string       _ip;
        /** the port, e.g. 8080 */
        unsigned int _port;
    };

}  // namespace support

