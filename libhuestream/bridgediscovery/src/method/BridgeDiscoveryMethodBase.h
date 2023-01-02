/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <memory>
#include <mutex>
#include <vector>

#include "boost/optional.hpp"

#include "bridgediscovery/BridgeDiscoveryResult.h"
#include "events/BridgeDiscoveryEventNotifier.h"
#include "events/BridgeDiscoveryEvents.h"
#include "method/IBridgeDiscoveryMethod.h"

#include "support/logging/Log.h"
#include "support/threading/Job.h"
#include "support/threading/QueueDispatcher.h"
#include "support/util/EventNotifierProvider.h"
#include "support/util/VectorOperations.h"
#include "support/util/Uuid.h"

namespace huesdk {
    inline BridgeDiscovery::Option convert(const BridgeDiscoveryClassType t) {
        switch (t) {
            case BRIDGE_DISCOVERY_CLASS_TYPE_UPNP :
                return BridgeDiscovery::Option::UPNP;
            case BRIDGE_DISCOVERY_CLASS_TYPE_IPSCAN :
                return BridgeDiscovery::Option::IPSCAN;
            case BRIDGE_DISCOVERY_CLASS_TYPE_NUPNP :
                return BridgeDiscovery::Option::NUPNP;
            case BRIDGE_DISCOVERY_CLASS_TYPE_MDNS:
                return BridgeDiscovery::Option::MDNS;
            default :
                HUE_LOG << HUE_ERROR
                        << "BridgeDiscoveryClassType can't be converted to BridgeDiscovery::Option" << HUE_ENDL;
        }
        return {};
    }
}  // namespace huesdk

namespace huesdk {
    template<typename TaskT>
    class BridgeDiscoveryMethodBase : public IBridgeDiscoveryMethod {
    public:
        using TaskType = TaskT;
        using MethodResultCallback = std::function<void(const std::vector<std::shared_ptr<BridgeDiscoveryResult>> &)>;

        BridgeDiscoveryMethodBase(
                const boost::uuids::uuid& request_id,
                const std::shared_ptr<IBridgeDiscoveryEventNotifier>& notifier);

        /**
         @see IBridgeDiscoveryMethod.h
        */
        void search(std::shared_ptr<IBridgeDiscoveryCallback> callback) final;

        /**
         @see IBridgeDiscoveryMethod.h
         */
        void stop() final;

        /**
         @see IBridgeDiscoveryMethod.h
         */
        bool is_searching() final;

        ~BridgeDiscoveryMethodBase() override {
            stop();
        }

    protected:
        /**
         Starts method search job
         @param callback The callback which will be called when method finishes searching or stops
                Contains the search results. Will be called only once.
         @return True when job started successfully, false otherwise
         */
        virtual bool method_search(const MethodResultCallback &callback) = 0;

        std::unique_ptr<support::Job<TaskType>> _job;

    protected:
        boost::uuids::uuid _request_id;
        std::shared_ptr<IBridgeDiscoveryEventNotifier> _bridge_discovery_event_notifier;

    private:
        std::mutex _mutex;
        std::shared_ptr<IBridgeDiscoveryCallback> _callback;
        support::QueueDispatcher _dispatcher;
        boost::optional<std::chrono::time_point<std::chrono::system_clock>> _start_of_search;
    };

    template <typename TaskType>
    BridgeDiscoveryMethodBase<TaskType>::BridgeDiscoveryMethodBase(
            const boost::uuids::uuid& request_id,
            const std::shared_ptr<IBridgeDiscoveryEventNotifier>& notifier)
            : _request_id(request_id), _bridge_discovery_event_notifier(notifier), _callback(nullptr)
    {}

    template<typename TaskType>
    bool BridgeDiscoveryMethodBase<TaskType>::is_searching() {
        std::lock_guard<std::mutex> lock_guard{_mutex};
        return _job != nullptr && _job->state() == support::JobState::Running;
    }

    template<typename TaskType>
    void BridgeDiscoveryMethodBase<TaskType>::search(std::shared_ptr<IBridgeDiscoveryCallback> callback) {
        std::lock_guard<std::mutex> lock_guard{_mutex};

        if (callback == nullptr) {
            HUE_LOG << HUE_CORE << HUE_WARN << "BridgeDiscoveryMethodBase: no callback defined " << HUE_ENDL;
            return;
        }

        _callback = callback;

        if (_job != nullptr) {
            _dispatcher.post([callback]() {
                (*callback)({}, BRIDGE_DISCOVERY_RETURN_CODE_BUSY);
            });
            return;
        }

        auto search_started = method_search(
                [this, callback](const std::vector<std::shared_ptr<BridgeDiscoveryResult>> &results) {
                    if (_bridge_discovery_event_notifier != nullptr) {
                        bridge_discovery_events::DiscoveryMethodFinished discovery_method_finished_event {
                                _request_id,
                                convert(get_type()),
                                std::chrono::system_clock::now() - _start_of_search.value(),
                                bridge_discovery_events::Status::SUCCESSFULLY_COMPLETED
                        };

                        _bridge_discovery_event_notifier->on_event(discovery_method_finished_event);
                    }

                    _dispatcher.post([callback, results]() {
                        (*callback)(results, BRIDGE_DISCOVERY_RETURN_CODE_SUCCESS);
                    });
                });

        if (!search_started) {
            _dispatcher.post([callback]() {
                (*callback)({}, BRIDGE_DISCOVERY_RETURN_CODE_BUSY);
            });
        } else {
            _start_of_search = {std::chrono::system_clock::now()};

            if (_bridge_discovery_event_notifier != nullptr) {
                bridge_discovery_events::DiscoveryMethodStarted discovery_method_started_event {
                        _request_id, convert(get_type())
                };
                _bridge_discovery_event_notifier->on_event(discovery_method_started_event);
            }
        }
    }

    template<typename TaskType>
    void BridgeDiscoveryMethodBase<TaskType>::stop() {
        std::lock_guard<std::mutex> lock_guard{_mutex};
        if (_job != nullptr && _job->cancel()) {
            if (_bridge_discovery_event_notifier != nullptr) {
                bridge_discovery_events::DiscoveryMethodFinished discovery_method_finished_event {
                        _request_id,
                        convert(get_type()),
                        std::chrono::system_clock::now() - _start_of_search.value(),
                        bridge_discovery_events::Status::CANCELLED
                };
                _bridge_discovery_event_notifier->on_event(discovery_method_finished_event);
            }

            _dispatcher.post([this] {
                (*_callback)(_job->get()->get_result(), BRIDGE_DISCOVERY_RETURN_CODE_STOPPED);
            });
        }
    }

}  // namespace huesdk

