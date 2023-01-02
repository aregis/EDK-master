/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <memory>
#include <set>
#include <map>
#include <string>

#include "method/mdns/IHueMDNSService.h"
#include "method/mdns/IMDNSResponder.h"

#include "support/threading/Thread.h"
#include "support/threading/QueueExecutor.h"

namespace huesdk {
    class HueMDNSService : public IHueMDNSService {
    public:
        constexpr static const char* HUE_REGTYPE = "_hue._tcp";
        constexpr static const char* HAP_REGTYPE = "_hap._tcp";
        constexpr static const char* LOCAL_DOMAIN = "local.";

        explicit HueMDNSService(std::shared_ptr<support::OperationalQueue> operational_queue = std::make_shared<support::OperationalQueue>(),
                std::chrono::milliseconds poll_timeout = std::chrono::milliseconds(1000));
        ~HueMDNSService();

        std::map<Host, HostInfo> get_hosts() const override;

        void on_browse_result_ready(DNSServiceRef ref, DNSServiceFlags flags, uint32_t,
                                    DNSServiceErrorType error, const char* name, const char* type,
                                    const char* domain);
        void on_resolve_result_ready(DNSServiceRef ref, DNSServiceFlags flags, uint32_t interface_index,
                                     DNSServiceErrorType error, const char *fullname, const char *host, uint16_t port,
                                     uint16_t txt_len,  const unsigned char *txt_record);
        void on_queryrecord_result_ready(DNSServiceRef ref, DNSServiceFlags flags, uint32_t interface_index, DNSServiceErrorType error,
                                          const char *fullname, uint16_t record_type, uint16_t record_class, uint16_t record_data_len,
                                          const void *record_data, uint32_t ttl);

    private:
        void start_browsing(const char* regtype, const char* domain);
        void deallocate_service_refs(std::set<DNSServiceRef>* service_refs);

        const std::chrono::milliseconds _poll_timeout;
        std::set<DNSServiceRef> _browse_service_refs;
        std::set<DNSServiceRef> _pending_service_refs;
        std::shared_ptr<IMDNSResponder> _mdns_responder_lib;
        std::chrono::steady_clock::time_point _start_time;
        std::map<IHueMDNSService::Host, IHueMDNSService::HostInfo> _hosts;
        support::QueueExecutor _queue_executor;
    };
}  // namespace huesdk