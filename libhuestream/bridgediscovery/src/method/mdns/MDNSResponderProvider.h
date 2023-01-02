/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <memory>

#include "method/mdns/IMDNSResponder.h"
#include "support/util/Provider.h"

namespace huesdk {
    using MDNSResponderProvider = support::Provider<std::shared_ptr<IMDNSResponder>>;
    using ScopedMDNSResponderProvider = support::ScopedProvider<std::shared_ptr<IMDNSResponder>>;

    std::shared_ptr<huesdk::IMDNSResponder> create_default_mdns_responder();
}  // namespace huesdk

template<>
struct default_object<std::shared_ptr<huesdk::IMDNSResponder>> {
    static std::shared_ptr<huesdk::IMDNSResponder> get() {
        return huesdk::create_default_mdns_responder();
    }
};