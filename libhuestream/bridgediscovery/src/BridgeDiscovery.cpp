/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <boost/lexical_cast.hpp>

#include "events/BridgeDiscoveryEvents.h"
#include "events/BridgeDiscoveryEventNotifier.h"
#include "bridgediscovery/BridgeDiscovery.h"
#include "method/BridgeDiscoveryMethodFactory.h"
#include "method/BridgeDiscoveryMethodUtil.h"
#include "tasks/BridgeDiscoveryTask.h"
#include "tasks/BridgeDiscoveryCheckIpArrayTask.h"

#include "support/logging/Log.h"
#include "support/util/Uuid.h"
#include "support/util/Factory.h"
#include "support/util/EventNotifierProvider.h"

using std::vector;
using std::unique_ptr;
using std::string;
using std::lock_guard;

using support::EventNotifierProvider;

class BridgeDiscoveryResult;

namespace huesdk {
    BridgeDiscovery::BridgeDiscovery() {
        auto event_notifier = EventNotifierProvider::get();
        if (event_notifier == boost::none) {
            HUE_LOG << HUE_CORE << HUE_WARN << "BridgeDiscovery: No event notifier set" << HUE_ENDL;
            event_notifier = [](const std::string&, const std::unordered_map<std::string, std::string>&) {};
        }
        _bridge_discovery_event_notifier = std::make_shared<BridgeDiscoveryEventNotifier>(event_notifier.value());
    }

    BridgeDiscovery::BridgeDiscovery(const std::shared_ptr<IBridgeDiscoveryEventNotifier>& notifier)
            : _bridge_discovery_event_notifier(notifier)
    {}

    BridgeDiscovery::~BridgeDiscovery() {
        HUE_LOG << HUE_CORE << HUE_DEBUG << "BridgeDiscovery: destructor" << HUE_ENDL;

        stop();
    }

    void BridgeDiscovery::search(IBridgeDiscoveryCallback *callback) {
        search(BridgeDiscovery::Option::MDNS | BridgeDiscovery::Option::NUPNP, callback);
    }

    void BridgeDiscovery::search(Callback callback) {
        search(BridgeDiscovery::Option::MDNS | BridgeDiscovery::Option::NUPNP, callback);
    }

    void BridgeDiscovery::search(support::EnumSet<Option> options, IBridgeDiscoveryCallback *callback) {
        if (callback == nullptr) {
            HUE_LOG << HUE_CORE << HUE_WARN << "BridgeDiscovery: no callback defined " << HUE_ENDL;
            return;
        }

        search(options, [callback](
                const std::vector<std::shared_ptr<BridgeDiscoveryResult>>& results, ReturnCode return_code) {
            (*callback)(results, static_cast<BridgeDiscoveryReturnCode>(return_code)); });
    }

    void BridgeDiscovery::search(support::EnumSet<Option> options, Callback callback) {
        lock_guard<decltype(_mutex)> lock(_mutex);

        if (!static_cast<bool>(callback)) {
            HUE_LOG << HUE_CORE << HUE_WARN << "BridgeDiscovery: empty callback provided " << HUE_ENDL;
            return;
        }

        if (is_searching()) {
            HUE_LOG << HUE_CORE << HUE_DEBUG
                    << "BridgeDiscovery: trying to start a search while a search is already in progress" << HUE_ENDL;

            _dispatcher.post([callback] {
                callback({}, ReturnCode::BUSY);
            });
            return;
        }

        _current_search_info = create_current_search_info();

        auto discovery_methods = get_discovery_methods(options);
        if (discovery_methods.empty()) {
            HUE_LOG << HUE_CORE << HUE_DEBUG << "BridgeDiscovery: no discovery methods provided" << HUE_ENDL;
            _dispatcher.post([callback] {
                callback({}, ReturnCode::MISSING_DISCOVERY_METHODS);
            });
            return;
        }

        if (_bridge_discovery_event_notifier != nullptr) {
            _bridge_discovery_event_notifier->on_event(
                    bridge_discovery_events::DiscoveryStarted{_current_search_info.value().first});
        }

        HUE_LOG << HUE_CORE << HUE_DEBUG << "BridgeDiscovery: start searching" << HUE_ENDL;
        _discovery_job = support::create_job<BridgeDiscoveryTask>(std::move(discovery_methods));
        _discovery_job->run([this, callback](BridgeDiscoveryTask *task) {
            auto results = task->get_results();

            if (_bridge_discovery_event_notifier != nullptr) {
                bridge_discovery_events::DiscoveryFinished discovery_finished_event{
                        _current_search_info.value().first,
                        std::chrono::system_clock::now() - _current_search_info.value().second,
                        bridge_discovery_events::Status::SUCCESSFULLY_COMPLETED
                };

                _bridge_discovery_event_notifier->on_event(discovery_finished_event);
            }

            _dispatcher.post([callback, results]() {
                callback(results, ReturnCode::SUCCESS);
            });
        });

        _callback = callback;
    }

    bool BridgeDiscovery::is_searching() {
        lock_guard<decltype(_mutex)> lock(_mutex);

        if (_discovery_job != nullptr) {
            return _discovery_job->state() == support::JobState::Running;
        }
        return false;
    }

    void BridgeDiscovery::stop() {
        lock_guard<decltype(_mutex)> lock(_mutex);

        HUE_LOG << HUE_CORE << HUE_DEBUG << "BridgeDiscovery: stopping search" << HUE_ENDL;

        if (_discovery_job != nullptr && _discovery_job->cancel()) {
            auto callback = _callback;
            auto result = _discovery_job->get()->get_results();

            if (_bridge_discovery_event_notifier != nullptr) {
                bridge_discovery_events::DiscoveryFinished discovery_finished_event{
                        _current_search_info.value().first,
                        std::chrono::system_clock::now() - _current_search_info.value().second,
                        bridge_discovery_events::Status::CANCELLED
                };

                _bridge_discovery_event_notifier->on_event(discovery_finished_event);
            }

            _dispatcher.post([callback, result] {
                callback(result, ReturnCode::STOPPED);
            });
        }
    }

    vector<unique_ptr<IBridgeDiscoveryMethod>> BridgeDiscovery::get_discovery_methods(support::EnumSet<Option> options) {
        vector<unique_ptr<IBridgeDiscoveryMethod>> discovery_methods;

        // NOTE: IPSCAN must go first, because its search will contain all needed bridge info. Possible following (N)UPNP search results of the same bridge will be discarded.
        auto discovery_options = {
                BridgeDiscovery::Option::MDNS,
                BridgeDiscovery::Option::IPSCAN,
                BridgeDiscovery::Option::UPNP,
                BridgeDiscovery::Option::NUPNP
        };

        for (const auto discovery_option : discovery_options) {
            if (options & discovery_option) {
                auto method = BridgeDiscoveryMethodFactory::create(
                        discovery_option, _current_search_info->first, _bridge_discovery_event_notifier);

                if (method != nullptr) {
                    discovery_methods.push_back(std::move(method));
                } else {
                    HUE_LOG << HUE_CORE << HUE_DEBUG << "BridgeDiscovery: nullptr provided as search method"
                            << static_cast<int>(discovery_option) << HUE_ENDL;
                }
            }
        }

        return discovery_methods;
    }

    BridgeDiscovery::InfoForCurrentSearch BridgeDiscovery::create_current_search_info() {
        static boost::uuids::random_generator generator;
        return {{generator(), std::chrono::system_clock::now()}};
    }

}  // namespace huesdk
