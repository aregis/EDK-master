/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <memory>

#include "method/mdns/IMDNSResponder.h"

#undef interface
#include "mdns_responder/mDNSCore/mDNSEmbeddedAPI.h"
#include "mdns_responder/mDNSShared/dns_sd.h"

namespace huesdk {

    class MDNSResponder : public IMDNSResponder {
    public:
        MDNSResponder();
        ~MDNSResponder();

        void poll(const std::chrono::milliseconds& timeout) override;

        DNSServiceErrorType browse(
                DNSServiceRef *sdRef,
                DNSServiceFlags flags,
                uint32_t interfaceIndex,
                const char *regtype,
                const char *domain,
                DNSServiceBrowseReply callBack,
                void *context) override;

        DNSServiceErrorType resolve(
                DNSServiceRef *sdRef,
                DNSServiceFlags flags,
                uint32_t interfaceIndex,
                const char *name,
                const char *regtype,
                const char *domain,
                DNSServiceResolveReply callBack,
                void *context) override;

        DNSServiceErrorType query_record(
                DNSServiceRef *sdRef,
                DNSServiceFlags flags,
                uint32_t interfaceIndex,
                const char *fullname,
                uint16_t rrtype,
                uint16_t rrclass,
                DNSServiceQueryRecordReply callBack,
                void *context) override;

        void deallocate_service_ref(DNSServiceRef sdRef) override;

    protected:
        mDNS _mDNS_storage = {};
        std::shared_ptr<mDNS_PlatformSupport> _platform_storage;

        constexpr static const int RR_CACHE_SIZE = 500;
        CacheEntity _RR_cache[RR_CACHE_SIZE];
    };

}  // namespace huesdk