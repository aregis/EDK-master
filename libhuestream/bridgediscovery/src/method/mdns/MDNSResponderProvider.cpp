/*******************************************************************************
  Copyright (C) 2019 Signify Holding
  All Rights Reserved.
 ********************************************************************************/

#include "method/mdns/MDNSResponderProvider.h"
#include "method/mdns/MDNSResponder.h"

std::shared_ptr<huesdk::IMDNSResponder> huesdk::create_default_mdns_responder() {
    return std::make_shared<huesdk::MDNSResponder>();
}