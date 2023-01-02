/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

namespace huesdk {
    
    typedef enum BridgeDiscoveryReturnCode {
        BRIDGE_DISCOVERY_RETURN_CODE_SUCCESS                       =  0,
        BRIDGE_DISCOVERY_RETURN_CODE_BUSY                          = -5,
        BRIDGE_DISCOVERY_RETURN_CODE_NULL_PARAMETER                = -101,
        BRIDGE_DISCOVERY_RETURN_CODE_STOPPED                       = -303,
        BRIDGE_DISCOVERY_RETURN_CODE_MISSING_DISCOVERY_METHODS     = -401,
    } BridgeDiscoveryReturnCode;
    
}  // namespace huesdk

