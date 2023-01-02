/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include "method/mdns/HueMDNSService.h"

#include <vector>
#include <utility>
#include <algorithm>

#if defined(_WIN32)
#   include <winsock2.h>
#   include <Ws2tcpip.h>
#else
#   include <arpa/inet.h>
#endif

#include "support/logging/Log.h"
#include "support/threading/RepetitiveTask.h"
#include "method/mdns/MDNSResponderProvider.h"

using huesdk::HueMDNSService;
using huesdk::IHueMDNSService;
using huesdk::MDNSResponderProvider;

static void DNSSD_API service_resolve_result_handler(DNSServiceRef ref, DNSServiceFlags flags, uint32_t interface_index,
                                  DNSServiceErrorType error, const char *fullname, const char *host, uint16_t port,
                                  uint16_t txt_len,  const unsigned char *txt_record, void *context) {
    auto mdns_service = reinterpret_cast<HueMDNSService*>(context);
    mdns_service->on_resolve_result_ready(ref, flags, interface_index, error, fullname, host, port, txt_len, txt_record);
}

static void DNSSD_API service_browse_result_handler(DNSServiceRef ref, DNSServiceFlags flags, uint32_t interface_index,
                            DNSServiceErrorType error, const char* name, const char* type,
                            const char* domain, void* context) {
    auto mdns_service = reinterpret_cast<HueMDNSService*>(context);
    mdns_service->on_browse_result_ready(ref, flags, interface_index, error, name, type, domain);
}

static void DNSSD_API service_queryrecord_result_handler(DNSServiceRef ref, DNSServiceFlags flags, uint32_t interface_index,
                                               DNSServiceErrorType error, const char *fullname,  uint16_t record_type,
                                               uint16_t record_class, uint16_t record_data_len, const void *record_data,
                                               uint32_t ttl, void *context) {
    auto mdns_service = reinterpret_cast<HueMDNSService*>(context);
    mdns_service->on_queryrecord_result_ready(ref, flags, interface_index, error, fullname, record_type, record_class, record_data_len, record_data, ttl);
}

void HueMDNSService::deallocate_service_refs(std::set<DNSServiceRef>* service_refs) {
    for (auto ref : *service_refs) {
        _mdns_responder_lib->deallocate_service_ref(ref);
    }
    service_refs->clear();
}

HueMDNSService::HueMDNSService(std::shared_ptr<support::OperationalQueue> operational_queue, std::chrono::milliseconds poll_timeout)
        : _poll_timeout{std::move(poll_timeout)}
        , _mdns_responder_lib{MDNSResponderProvider::get()}
        , _queue_executor{std::move(operational_queue)} {
    _start_time = std::chrono::steady_clock::now();
    _hosts = {};

    start_browsing(HUE_REGTYPE, LOCAL_DOMAIN);
    start_browsing(HAP_REGTYPE, LOCAL_DOMAIN);

    if (_browse_service_refs.size() != 0) {
        _queue_executor.execute(support::RepetitiveTask(&_queue_executor, [this](){
            _mdns_responder_lib->poll(_poll_timeout);
        }));
    }

    HUE_LOG << HUE_CORE << HUE_DEBUG << "HueMDNSService started." << HUE_ENDL;
}

HueMDNSService::~HueMDNSService() {
    _queue_executor.shutdown();

    deallocate_service_refs(&_browse_service_refs);
    deallocate_service_refs(&_pending_service_refs);

    HUE_LOG << HUE_CORE << HUE_DEBUG << "HueMDNSService stopped." << HUE_ENDL;
}

void HueMDNSService::start_browsing(const char* regtype, const char* domain) {
    DNSServiceRef ref = {};
    DNSServiceErrorType err = _mdns_responder_lib->browse(&ref, kDNSServiceFlagsBrowseDomains, 0, regtype, domain, service_browse_result_handler, this);
    if (kDNSServiceErr_NoError == err) {
        _browse_service_refs.insert(ref);
    } else {
        HUE_LOG << HUE_CORE << HUE_WARN << "Unable start browsing " << regtype << " [error: " << err << "]" << HUE_ENDL;
    }
}

void HueMDNSService::on_browse_result_ready(DNSServiceRef ref, DNSServiceFlags flags, uint32_t interface_index,
                            DNSServiceErrorType error, const char* name, const char* type,
                            const char* domain) {
    if (error == kDNSServiceErr_NoError) {
        HUE_LOG << HUE_CORE << HUE_DEBUG << "HueMDNSService browse result ready "
                << "[ref: " << ref << ", flags:" << flags << ", interface:" << interface_index
                << ", name:" << name << ", type:" << type << ", domain:" << domain << "]" << HUE_ENDL;

        if (flags & kDNSServiceFlagsAdd) {
            DNSServiceRef resolve_service_ref = {};
            auto err = _mdns_responder_lib->resolve(&resolve_service_ref, (DNSServiceFlags) 0, 0, name, type, domain,
                                                    &service_resolve_result_handler, this);

            if (kDNSServiceErr_NoError == err) {
                _pending_service_refs.insert(resolve_service_ref);
            }
        }
    } else {
        HUE_LOG << HUE_CORE << HUE_WARN << "HueMDNSService browse failure [ref: " << ref << ", error: " << error << "]" << HUE_ENDL;
    }
}

void HueMDNSService::on_resolve_result_ready(DNSServiceRef ref, DNSServiceFlags flags, uint32_t interface_index,
                             DNSServiceErrorType error, const char *, const char *host, uint16_t port,
                             uint16_t txt_len, const unsigned char * txt_record) {
    (void) txt_record;

    if (error == kDNSServiceErr_NoError) {
        HUE_LOG << HUE_CORE << HUE_DEBUG << "HueMDNSService resolve result ready "
                << "[ref: " << ref << ", flags:" << flags << ", interface:" << interface_index
                << ", host:" << host << ", port:" << port << ", txt_len:" << txt_len << "]" << HUE_ENDL;

        DNSServiceRef queryrecord_service_ref = {};
        auto err = _mdns_responder_lib->query_record(&queryrecord_service_ref, (DNSServiceFlags) 0, interface_index,
                                                     host,
                                                     kDNSServiceType_A, kDNSServiceClass_IN,
                                                     service_queryrecord_result_handler, this);

        if (kDNSServiceErr_NoError == err) {
            _pending_service_refs.insert(queryrecord_service_ref);
        }
    } else {
        HUE_LOG << HUE_CORE << HUE_WARN << "HueMDNSService resolve failure [ref: " << ref << ", error: " << error << "]" << HUE_ENDL;
    }
}

void HueMDNSService::on_queryrecord_result_ready(DNSServiceRef ref, DNSServiceFlags flags, uint32_t interface_index, DNSServiceErrorType error,
                                  const char *fullname, uint16_t record_type, uint16_t record_class, uint16_t record_data_len,
                                  const void *record_data, uint32_t ttl) {
    if (error == kDNSServiceErr_NoError) {
        HUE_LOG << HUE_CORE << HUE_DEBUG << "HueMDNSService queryrecord result ready "
                << "[ref: " << ref << ", flags:" << flags << ", interface:" << interface_index
                << ", fullname:" << fullname << ", record_type:" << record_type << ", record_class:" << record_class
                << ", record_data_len: " << record_data_len << ", ttl: " << ttl << "]" << HUE_ENDL;

        if (flags & kDNSServiceFlagsAdd) {
            char host_ip[256];
            *host_ip = 0;

            std::vector<char> record_data_vector(record_data_len);
            std::copy(static_cast<const char*>(record_data), static_cast<const char*>(record_data) + record_data_len,
                      record_data_vector.data());

            inet_ntop(AF_INET, record_data_vector.data(), host_ip, sizeof(host_ip));
            auto& host = _hosts[fullname];
            host.ip_address = host_ip;
            host.discovery_duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - _start_time);
        }
    } else {
        HUE_LOG << HUE_CORE << HUE_WARN << "HueMDNSService queryrecord failure [ref: << " << ref << ", error: " << error << "]" << HUE_ENDL;
    }
}

std::map<IHueMDNSService::Host, IHueMDNSService::HostInfo> HueMDNSService::get_hosts() const {
    return _hosts;
}