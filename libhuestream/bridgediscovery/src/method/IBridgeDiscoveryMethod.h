/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <memory>
#include "bridgediscovery/BridgeDiscoveryClassType.h"
#include "bridgediscovery/BridgeDiscoveryReturnCode.h"
#include "bridgediscovery/IBridgeDiscoveryCallback.h"

namespace huesdk {
    class IBridgeDiscoveryMethod {
    public:
        /**
         Constructor
         */
        IBridgeDiscoveryMethod() = default;

        /** 
         Destructor
         */
        virtual ~IBridgeDiscoveryMethod() = default;

        /** 
         Start searching for bridges. The search will be performed asynchronously and the callback
         will be called from global dispatching queue.
         @param  callback The callback which will be called when the search is finished and provides
                 all the found results. The callback will only be called once
         */

        virtual void search(std::shared_ptr<IBridgeDiscoveryCallback> callback) = 0;

        /**
         Whether a search is still in progress
         @return true when searching, false otherwise
         */
        virtual bool is_searching() = 0;

        /**
         Stop searching if still in progress. This method will block until the search is completely stopped.
         It's also guaranteed, if a search was still in progress, that the callback will be called with the
         found results.
         */
        virtual void stop() = 0;

        /**
         Returns the type of this class as an enumeration, which is unique for
         every single class
         @see    BridgeDiscoveryClassType.h
         @return The type of this class as integer
         */
        virtual BridgeDiscoveryClassType get_type() const = 0;
    };

}  // namespace huesdk

