/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <memory>
#include <vector>

#include "support/util/NonCopyableBase.h"
#include "support/util/EnumSet.h"

#include "bridgediscovery/IBridgeDiscoveryCallback.h"
#include "bridgediscovery/BridgeDiscoveryReturnCode.h"

namespace huesdk {

    class IBridgeDiscovery : support::NonCopyableBase {
    public:
        enum class Option {
            UPNP   = 1,        ///< DEPRECATED (superseded by MDNS). search for bridges via UPnP on the local network
            IPSCAN = 1 << 1,   ///< brute force scanning for bridges on the local network.
            ///< Scans only the last subnet of the ip (IPV4 only).
            ///< If multiple network interfaces are present it picks the first one in the list */
            NUPNP = 1 << 2,   ///< search for bridges via the portal
            MDNS = 1 << 3
        };

        enum class ReturnCode {
            SUCCESS                       =  0,    ///< search was successful
            BUSY                          = -5,    ///< a search is already in progress, it's not allowed to start multiple searches simultaneously
            NULL_PARAMETER                = -101,
            STOPPED                       = -303,  ///< search has been stopped
            MISSING_DISCOVERY_METHODS     = -401,  ///< this indicates no discovery methods could be found
        };

        using Callback = std::function<void(std::vector<std::shared_ptr<BridgeDiscoveryResult>>, ReturnCode)>;

        /**
         Start searching for bridges. Since no options can be provided, only the UPnP and NUPnP discovery methods will
         be executed. The search will be performed in a different thread and also the callback will be called from that
         specific thread. This method is asynchronous and returns immediately
         @param  callback The callback which will be called when the search is finished and provides
                          all the found results. The callback will only be called once
         */
        virtual void search(Callback) = 0;

        /**
         Start searching for bridges. Which of the available discovery methods should be executed, can be set
         with the options parameter. The search will be performed in a different thread and also the callback will
         be called from that specific thread. This method is asynchronous and returns immediately
         @param  options Which of the discovery methods should be executed.
         @param  callback The callback which will be called when the search is finished and provides
                          all the found results. The callback will only be called once
         */
        virtual void search(support::EnumSet<Option> options, Callback) = 0;

        /**
         Whether a search is in progress
         */
        virtual bool is_searching() = 0;

        /**
         Stop searching if still in progress. This method will block until the search is completely stopped.
         It's also guaranteed, if a search was still in progress, that the callback will be called with the
         found results.
         */
        virtual void stop() = 0;
    };

}  // namespace huesdk

