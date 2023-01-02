/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <regex>
#include <string>

#include "support/util/StringUtil.h"

namespace huesdk {
    namespace bridge_discovery_const {

        /* upnp */

        /** the udp receive timeout in seconds */
        const unsigned int UDP_TIMEOUT = 8000;
        /** the ip and port where the discovery message will be sent to */
        const std::string UDP_SOCKET_IP = "239.255.255.250";
        const unsigned int UDP_SOCKET_PORT = 1900;
        /** size of the buffere where the received data will be stored */
        const unsigned int UDP_SOCKET_BUFFER_SIZE = 512;
        /** the discovery message */
        const std::string UPNP_REQUEST_MESSAGE = "M-SEARCH * HTTP/1.1\r\n"\
                                                     "HOST:" + UDP_SOCKET_IP + ":" + std::to_string(UDP_SOCKET_PORT) + "\r\n"\
                                                     "ST:ssdp:all\r\n"\
                                                     "Man:\"ssdp:discover\"\r\n"\
                                                     "MX:3\r\n\r\n";
        /** regex for parsing the found bridges from the received data */
        static const auto UPNP_RESPONSE_REGEX = std::regex("LOCATION: http://(.+):.+\\r\\n"\
                                                     "SERVER:.+IpBridge/(.+)\\r\\n"\
                                                     "ST:.+\\r\\n"\
                                                     "USN: uuid:.+-([0-9a-zA-Z]{12})::upnp:rootdevice(\\r\\nhue-bridgeid:\\s*([0-9a-zA-Z]*))?");
        /** regex for parsing the found bridges for bridges with firmware >= 1.9 */
        static const auto UPNP_RESPONSE_REGEX_19 = std::regex("LOCATION: http://(.+):.+\\r\\n"\
                                                     "SERVER:.+IpBridge/(.+)\\r\\n"\
                                                     "hue-bridgeid:\\s*([0-9a-zA-Z].+)\\r\\n");

        /* nupnp */

        /** portal url to get the bridges, which is by default meethue */
        const std::string NUPNP_HTTP_URL_PATH = "/api/nupnp";
        /** connection timeout of the http request in seconds */
        const unsigned int NUPNP_HTTP_CONNECT_TIMEOUT = 8;
        const unsigned int NUPNP_HTTP_RECEIVE_TIMEOUT = 8;
        const unsigned int NUPNP_HTTP_REQUEST_TIMEOUT = 15;

        /* ipscan */
        /** which part of the last subnet of the ip address which should be scanned */
        const unsigned int IPSCAN_IP_RANGE_BEGIN = 1;
        const unsigned int IPSCAN_IP_RANGE_END = 254;

#ifdef WIN32
        // windows cannot handle too many open connections at once
        const unsigned int IPSCAN_MAX_ACTIVE_SOCKETS   = 100;
#else
        const unsigned int IPSCAN_MAX_ACTIVE_SOCKETS = 1024;
#endif

        /* ipcheck */

        const unsigned int IPCHECK_HTTP_PORT = 80;

        /** connection timeout of the http request */
        const unsigned int IPCHECK_HTTP_CONNECT_TIMEOUT = 2;
        const unsigned int IPCHECK_HTTP_RECEIVE_TIMEOUT = 2;
        const unsigned int IPCHECK_HTTP_REQUEST_TIMEOUT = 15;

        // interval between checks if search (ip scanning) was interrupted by user
        const unsigned int IPCHECK_CHECK_USER_STOP_TIMEOUT = 1000;  // in milliseconds

        /** url for getting bridge configuration from a bridge */
        const std::string IPCHECK_BRIDGE_CONFIG_HTTP_URL_PATH = "/api/nouser/config";

    }  // namespace bridge_discovery_const
}  // namespace huesdk

