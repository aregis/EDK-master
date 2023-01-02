/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <memory>
#include <mutex>
#include <type_traits>
#include <utility>
#include <vector>

#include "boost/optional.hpp"

#include "bridgediscovery/IBridgeDiscovery.h"
#include "bridgediscovery/IBridgeDiscoveryCallback.h"
#include "bridgediscovery/BridgeDiscoveryReturnCode.h"
#include "support/threading/Job.h"
#include "support/threading/QueueDispatcher.h"
#include "support/util/EnumSet.h"
#include "support/util/Uuid.h"

namespace huesdk {

    class IBridgeDiscoveryMethod;
    class BridgeDiscoveryTask;
    class IBridgeDiscoveryEventNotifier;

    class BridgeDiscovery : public IBridgeDiscovery {
    public:
        BridgeDiscovery();
        ~BridgeDiscovery() override;

        /** 
         Start searching for bridges. Since no options can be provided, only the UPnP and NUPnP discovery methods will
         be executed. The search will be performed in a different thread and also the callback will be called from that
         specific thread. This method is asynchronous and returns immediately
         @param  callback The callback which will be called when the search is finished and provides
                          all the found results. The callback will only be called once
         */
        void search(IBridgeDiscoveryCallback *callback);

        void search(Callback) override;

        /** 
         Start searching for bridges. Which of the available discovery methods should be executed, can be set
         with the options parameter. The search will be performed in a different thread and also the callback will
         be called from that specific thread. This method is asynchronous and returns immediately
         @param  options Which of the discovery methods should be executed.
         @param  callback The callback which will be called when the search is finished and provides
                          all the found results. The callback will only be called once
         */
        void search(support::EnumSet<Option> options, IBridgeDiscoveryCallback *callback);

        void search(support::EnumSet<Option> options, Callback) override;

        /**
         Whether a search is in progress
         */
        bool is_searching() override;

        /**
         Stop searching if still in progress. This method will block until the search is completely stopped.
         It's also guaranteed, if a search was still in progress, that the callback will be called with the
         found results.
         */
        void stop() override;

    protected:
        explicit BridgeDiscovery(const std::shared_ptr<IBridgeDiscoveryEventNotifier>& notifier);

    private:
        std::recursive_mutex _mutex;
        std::shared_ptr<support::Job<BridgeDiscoveryTask>> _discovery_job;
        Callback _callback;
        support::QueueDispatcher _dispatcher;
        std::shared_ptr<IBridgeDiscoveryEventNotifier> _bridge_discovery_event_notifier;

        using InfoForCurrentSearch
            = boost::optional<std::pair<boost::uuids::uuid, std::chrono::time_point<std::chrono::system_clock>>>;

        InfoForCurrentSearch _current_search_info;

        std::vector<std::unique_ptr<IBridgeDiscoveryMethod>> get_discovery_methods(support::EnumSet<Option> options);
        InfoForCurrentSearch create_current_search_info();
    };

    inline support::EnumSet<BridgeDiscovery::Option> operator|(BridgeDiscovery::Option lhs, BridgeDiscovery::Option rhs) {
        return {lhs, rhs};
    }

}  // namespace huesdk

