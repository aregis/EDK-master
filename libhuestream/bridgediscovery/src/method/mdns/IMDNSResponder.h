/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <chrono>

#include "support/util/NonCopyableBase.h"

#undef interface
#include "mdns_responder/mDNSCore/mDNSEmbeddedAPI.h"
#include "mdns_responder/mDNSShared/dns_sd.h"

namespace huesdk {

    class IMDNSResponder : support::NonCopyableBase {
    public:
        virtual void poll(const std::chrono::milliseconds& timeout) = 0;

        virtual DNSServiceErrorType browse(
                        DNSServiceRef *sdRef,
                        DNSServiceFlags flags,
                        uint32_t interfaceIndex,
                        const char *regtype,
                        const char *domain,
                        DNSServiceBrowseReply callBack,
                        void *context) = 0;

        virtual DNSServiceErrorType resolve(
                        DNSServiceRef *sdRef,
                        DNSServiceFlags flags,
                        uint32_t interfaceIndex,
                        const char *name,
                        const char *regtype,
                        const char *domain,
                        DNSServiceResolveReply callBack,
                        void *context) = 0;

        virtual DNSServiceErrorType query_record(
                        DNSServiceRef *sdRef,
                        DNSServiceFlags flags,
                        uint32_t interfaceIndex,
                        const char *fullname,
                        uint16_t rrtype,
                        uint16_t rrclass,
                        DNSServiceQueryRecordReply callBack,
                        void *context) = 0;

        virtual void deallocate_service_ref(DNSServiceRef sdRef) = 0;
    };
}  // namespace huesdk