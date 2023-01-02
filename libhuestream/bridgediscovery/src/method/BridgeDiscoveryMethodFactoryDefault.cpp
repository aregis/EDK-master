/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <memory>

#include "bridgediscovery/BridgeDiscovery.h"
#include "BridgeDiscoveryMethodFactory.h"
#include "events/IBridgeDiscoveryEventNotifier.h"
#include "method/IBridgeDiscoveryMethod.h"
#include "method/ipscan/BridgeDiscoveryIpscan.h"
#include "method/mdns/BridgeDiscoveryMDNS.h"
#include "method/nupnp/BridgeDiscoveryNupnp.h"
#include "method/upnp/BridgeDiscoveryUpnp.h"
#include "support/util/MakeUnique.h"
#include "support/util/Uuid.h"

using std::unique_ptr;

template<>
std::unique_ptr<huesdk::IBridgeDiscoveryMethod>
huesdk_lib_default_factory<huesdk::IBridgeDiscoveryMethod, huesdk::BridgeDiscovery::Option,
   boost::uuids::uuid, std::shared_ptr<huesdk::IBridgeDiscoveryEventNotifier>>(
        huesdk::BridgeDiscovery::Option option,
        boost::uuids::uuid request_id,
        std::shared_ptr<huesdk::IBridgeDiscoveryEventNotifier> notifier) {
    switch (option) {
        case huesdk::BridgeDiscovery::Option::UPNP:
            return support::make_unique<huesdk::BridgeDiscoveryUpnp>(std::move(request_id), notifier);
        case huesdk::BridgeDiscovery::Option::NUPNP:
            return support::make_unique<huesdk::BridgeDiscoveryNupnp>(std::move(request_id), notifier);
        case huesdk::BridgeDiscovery::Option::IPSCAN:
            return support::make_unique<huesdk::BridgeDiscoveryIpscan>(std::move(request_id), notifier);
        case huesdk::BridgeDiscovery::Option::MDNS:
            return support::make_unique<huesdk::BridgeDiscoveryMDNS>(std::move(request_id), notifier);
    }
    return nullptr;
}