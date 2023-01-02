/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <memory>

#include "bridgediscovery/BridgeDiscovery.h"
#include "events/IBridgeDiscoveryEventNotifier.h"
#include "method/IBridgeDiscoveryMethod.h"
#include "support/util/Factory.h"
#include "support/util/Uuid.h"

namespace huesdk {

    using BridgeDiscoveryMethodFactory =
        support::Factory<std::unique_ptr<huesdk::IBridgeDiscoveryMethod>(
           huesdk::BridgeDiscovery::Option, boost::uuids::uuid, std::shared_ptr<IBridgeDiscoveryEventNotifier>)>;

    using ScopedBridgeDiscoveryMethodFactory =
        support::ScopedFactory<std::unique_ptr<huesdk::IBridgeDiscoveryMethod>(
           huesdk::BridgeDiscovery::Option, boost::uuids::uuid, std::shared_ptr<IBridgeDiscoveryEventNotifier>)>;

}  // namespace huesdk