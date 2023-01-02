/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

namespace huesdk {

    typedef enum BridgeDiscoveryClassType {
        BRIDGE_DISCOVERY_CLASS_TYPE_UNDEFINED = 0,
        BRIDGE_DISCOVERY_CLASS_TYPE_UPNP   = 1,
        BRIDGE_DISCOVERY_CLASS_TYPE_NUPNP  = 2,
        BRIDGE_DISCOVERY_CLASS_TYPE_IPSCAN = 3,
        BRIDGE_DISCOVERY_CLASS_TYPE_MDNS = 4
    }  BridgeDiscoveryClassType;

}  // namespace huesdk

