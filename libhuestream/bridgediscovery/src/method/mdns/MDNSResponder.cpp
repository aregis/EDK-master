/*******************************************************************************
  Copyright (C) 2019 Signify Holding
  All Rights Reserved.
 ********************************************************************************/

#include "method/mdns/MDNSResponder.h"
#include "method/mdns/MDNSResponderDefs.h"
#include "method/mdns/MDNSResponderPlatformUtilities.h"

using huesdk::MDNSResponder;

MDNSResponder::MDNSResponder()
        : _platform_storage{MDNSResponderPlatformUtilities::create_mdns_platform_support()} {
    ::mDNS_Init(&_mDNS_storage, _platform_storage.get(),
              _RR_cache, RR_CACHE_SIZE,
              mDNS_Init_DontAdvertiseLocalAddresses,
              mDNS_Init_NoInitCallback,
              mDNS_Init_NoInitCallbackContext);
    MDNSResponderPlatformUtilities::init_mdns(&_mDNS_storage);
}

MDNSResponder::~MDNSResponder() {
    ::mDNS_StartExit(&_mDNS_storage);
    ::mDNS_FinalExit(&_mDNS_storage);
}

void MDNSResponder::poll(const std::chrono::milliseconds& timeout) {
    struct timeval op_timeout;
    op_timeout.tv_sec = static_cast<decltype(op_timeout.tv_sec)>(timeout.count()) / 1000;
    op_timeout.tv_usec = (timeout.count() % 1000) * 1000;

    int nfds = 0;
    fd_set readfds;
    FD_ZERO(&readfds);
    MDNSResponderPlatformUtilities::get_fd_set(&_mDNS_storage, &nfds, &readfds, &op_timeout);

    if (nfds > 0) {
        if (SELECT(nfds + 1, &readfds, nullptr, nullptr, &op_timeout) != -1) {
            MDNSResponderPlatformUtilities::process_fd_set(&_mDNS_storage, &readfds);
        }
    }
}

//*************************************************************************************************************
// Browse for services

static void DNSServiceBrowseDispose(mDNS_DirectOP *op) {
    mDNS_DirectOP_Browse *x = reinterpret_cast<mDNS_DirectOP_Browse*>(op);
    ContextExtension* context_ext = reinterpret_cast<ContextExtension*>(x->context);
    mDNS_StopBrowse(context_ext->mDNSStorage, &x->q);
    delete x;
    delete context_ext;
}

static void FoundInstance(mDNS *const m, DNSQuestion *question, const ResourceRecord *const answer, QC_result AddRecord) {
    DNSServiceFlags flags = AddRecord ? kDNSServiceFlagsAdd : kDNSServiceFlagsPrivateOne;
    domainlabel name;
    domainname type, domain;
    char cname[MAX_DOMAIN_LABEL+1];         // Unescaped name: up to 63 bytes plus C-string terminating NULL.
    char ctype[MAX_ESCAPED_DOMAIN_NAME];
    char cdom[MAX_ESCAPED_DOMAIN_NAME];
    mDNS_DirectOP_Browse *x = reinterpret_cast<mDNS_DirectOP_Browse*>(question->QuestionContext);
    (void)m;        // Unused

    if (answer->rrtype != kDNSType_PTR)
    { LogMsg("FoundInstance: Should not be called with rrtype %d (not a PTR record)", answer->rrtype); return; }

    if (!DeconstructServiceName(&answer->rdata->u.name, &name, &type, &domain)) {
        LogMsg("FoundInstance: %##s PTR %##s received from network is not valid DNS-SD service pointer",
               answer->name->c, answer->rdata->u.name.c);
        return;
    }

    ConvertDomainLabelToCString_unescaped(&name, cname);
    ConvertDomainNameToCString(&type, ctype);
    ConvertDomainNameToCString(&domain, cdom);
    if (x->callback) {
        ContextExtension* context_ext = reinterpret_cast<ContextExtension*>(x->context);
        x->callback((DNSServiceRef)x, flags, 0, 0, cname, ctype, cdom, context_ext->context);
    }
}

DNSServiceErrorType MDNSResponder::browse(DNSServiceRef *sdRef, DNSServiceFlags flags, uint32_t interfaceIndex,
                                          const char *regtype, const char *domain,
                                          DNSServiceBrowseReply callback, void *context) {
    mStatus err = mStatus_NoError;
    domainname t, d;
    mDNS_DirectOP_Browse *x;
    (void)flags;            // Unused
    (void)interfaceIndex;   // Unused

    // Check parameters
    if (!regtype[0] || !MakeDomainNameFromDNSNameString(&t, regtype))      { return mStatus_BadParamErr; }
    if (!MakeDomainNameFromDNSNameString(&d, *domain ? domain : "local.")) { return mStatus_BadParamErr; }

    // Allocate memory, and handle failure
    x = new mDNS_DirectOP_Browse{};
    if (!x) { return mStatus_NoMemoryErr; }

    ContextExtension* context_ext = new ContextExtension;
    context_ext->context = context;
    context_ext->mDNSStorage = &_mDNS_storage;
    // Set up object
    x->disposefn = DNSServiceBrowseDispose;
    x->callback  = callback;
    x->context   = context_ext;
    x->q.QuestionContext = x;

    // Do the operation
    err = mDNS_StartBrowse(&_mDNS_storage, &x->q, &t, &d, mDNSNULL, mDNSInterface_Any, flags, (flags & kDNSServiceFlagsForceMulticast) != 0, (flags & kDNSServiceFlagsBackgroundTrafficClass) != 0, FoundInstance, x);
    if (err) {
        delete x;
        return err;
    }

    // Succeeded: Wrap up and return
    *sdRef = (DNSServiceRef)x;
    return(mStatus_NoError);
}

//*************************************************************************************************************
// Resolve Service Info

static void DNSServiceResolveDispose(mDNS_DirectOP *op) {
    mDNS_DirectOP_Resolve *x = reinterpret_cast<mDNS_DirectOP_Resolve*>(op);
    ContextExtension* context_ext = reinterpret_cast<ContextExtension*>(x->context);
    if (x->qSRV.ThisQInterval >= 0) mDNS_StopQuery(context_ext->mDNSStorage, &x->qSRV);
    if (x->qTXT.ThisQInterval >= 0) mDNS_StopQuery(context_ext->mDNSStorage, &x->qTXT);
    delete x;
    delete context_ext;
}

static void FoundServiceInfo(mDNS *const m, DNSQuestion *question, const ResourceRecord *const answer, QC_result AddRecord) {
    mDNS_DirectOP_Resolve *x = reinterpret_cast<mDNS_DirectOP_Resolve*>(question->QuestionContext);
    (void)m;    // Unused
    if (!AddRecord) {
        if (answer->rrtype == kDNSType_SRV && x->SRV == answer) x->SRV = mDNSNULL;
        if (answer->rrtype == kDNSType_TXT && x->TXT == answer) x->TXT = mDNSNULL;
    } else {
        if (answer->rrtype == kDNSType_SRV) x->SRV = answer;
        if (answer->rrtype == kDNSType_TXT) x->TXT = answer;
        if (x->SRV && x->TXT && x->callback) {
            char fullname[MAX_ESCAPED_DOMAIN_NAME], targethost[MAX_ESCAPED_DOMAIN_NAME];
            ConvertDomainNameToCString(answer->name, fullname);
            ConvertDomainNameToCString(&x->SRV->rdata->u.srv.target, targethost);
            ContextExtension* context_ext = reinterpret_cast<ContextExtension*>(x->context);
            x->callback((DNSServiceRef)x, 0, 0, kDNSServiceErr_NoError, fullname, targethost,
                        x->SRV->rdata->u.srv.port.NotAnInteger, x->TXT->rdlength, (unsigned char*)x->TXT->rdata->u.txt.c, context_ext->context);
        }
    }
}

DNSServiceErrorType MDNSResponder::resolve(DNSServiceRef *sdRef, DNSServiceFlags flags, uint32_t interfaceIndex,
                                           const char *name, const char *regtype, const char *domain,
                                           DNSServiceResolveReply callback, void *context) {
    mStatus err = mStatus_NoError;
    domainlabel n;
    domainname t, d, srv;
    mDNS_DirectOP_Resolve *x;

    (void)flags;            // Unused
    (void)interfaceIndex;   // Unused

    // Check parameters
    if (!name[0]    || !MakeDomainLabelFromLiteralString(&n, name  )) { return mStatus_BadParamErr; }
    if (!regtype[0] || !MakeDomainNameFromDNSNameString(&t, regtype)) { return mStatus_BadParamErr; }
    if (!domain[0]  || !MakeDomainNameFromDNSNameString(&d, domain )) { return mStatus_BadParamErr; }
    if (!ConstructServiceName(&srv, &n, &t, &d))                      { return mStatus_BadParamErr; }

    // Allocate memory, and handle failure
    x = new mDNS_DirectOP_Resolve{};
    if (!x) { return mStatus_NoMemoryErr; }

    ContextExtension* context_ext = new ContextExtension;
    context_ext->context = context;
    context_ext->mDNSStorage = &_mDNS_storage;

    // Set up object
    x->disposefn = DNSServiceResolveDispose;
    x->callback  = callback;
    x->context   = context_ext;
    x->SRV       = mDNSNULL;
    x->TXT       = mDNSNULL;

    x->qSRV.ThisQInterval       = -1;       // So that DNSServiceResolveDispose() knows whether to cancel this question
    x->qSRV.InterfaceID         = mDNSInterface_Any;
    x->qSRV.flags               = 0;
    x->qSRV.Target              = zeroAddr;
    AssignDomainName(&x->qSRV.qname, &srv);
    x->qSRV.qtype               = kDNSType_SRV;
    x->qSRV.qclass              = kDNSClass_IN;
    x->qSRV.LongLived           = mDNSfalse;
    x->qSRV.ExpectUnique        = mDNStrue;
    x->qSRV.ForceMCast          = mDNSfalse;
    x->qSRV.ReturnIntermed      = mDNSfalse;
    x->qSRV.SuppressUnusable    = mDNSfalse;
    x->qSRV.SearchListIndex     = 0;
    x->qSRV.AppendSearchDomains = 0;
    x->qSRV.RetryWithSearchDomains = mDNSfalse;
    x->qSRV.TimeoutQuestion     = 0;
    x->qSRV.WakeOnResolve       = 0;
    x->qSRV.UseBackgroundTrafficClass = (flags & kDNSServiceFlagsBackgroundTrafficClass) != 0;
    x->qSRV.ValidationRequired  = 0;
    x->qSRV.ValidatingResponse  = 0;
    x->qSRV.ProxyQuestion       = 0;
    x->qSRV.qnameOrig           = mDNSNULL;
    x->qSRV.AnonInfo            = mDNSNULL;
    x->qSRV.pid                 = mDNSPlatformGetPID();
    x->qSRV.QuestionCallback    = FoundServiceInfo;
    x->qSRV.QuestionContext     = x;

    x->qTXT.ThisQInterval       = -1;       // So that DNSServiceResolveDispose() knows whether to cancel this question
    x->qTXT.InterfaceID         = mDNSInterface_Any;
    x->qTXT.flags               = 0;
    x->qTXT.Target              = zeroAddr;
    AssignDomainName(&x->qTXT.qname, &srv);
    x->qTXT.qtype               = kDNSType_TXT;
    x->qTXT.qclass              = kDNSClass_IN;
    x->qTXT.LongLived           = mDNSfalse;
    x->qTXT.ExpectUnique        = mDNStrue;
    x->qTXT.ForceMCast          = mDNSfalse;
    x->qTXT.ReturnIntermed      = mDNSfalse;
    x->qTXT.SuppressUnusable    = mDNSfalse;
    x->qTXT.SearchListIndex     = 0;
    x->qTXT.AppendSearchDomains = 0;
    x->qTXT.RetryWithSearchDomains = mDNSfalse;
    x->qTXT.TimeoutQuestion     = 0;
    x->qTXT.WakeOnResolve       = 0;
    x->qTXT.UseBackgroundTrafficClass = (flags & kDNSServiceFlagsBackgroundTrafficClass) != 0;
    x->qTXT.ValidationRequired  = 0;
    x->qTXT.ValidatingResponse  = 0;
    x->qTXT.ProxyQuestion       = 0;
    x->qTXT.qnameOrig           = mDNSNULL;
    x->qTXT.AnonInfo            = mDNSNULL;
    x->qTXT.pid                 = mDNSPlatformGetPID();
    x->qTXT.QuestionCallback    = FoundServiceInfo;
    x->qTXT.QuestionContext     = x;

    err = mDNS_StartQuery(&_mDNS_storage, &x->qSRV);
    if (err) { DNSServiceResolveDispose(reinterpret_cast<mDNS_DirectOP*>(x)); return err; }
    err = mDNS_StartQuery(&_mDNS_storage, &x->qTXT);
    if (err) { DNSServiceResolveDispose(reinterpret_cast<mDNS_DirectOP*>(x)); return err; }

    // Succeeded: Wrap up and return
    *sdRef = (DNSServiceRef)x;
    return(mStatus_NoError);
}

//*************************************************************************************************************
// DNSServiceQueryRecord

static void DNSServiceQueryRecordDispose(mDNS_DirectOP *op) {
    mDNS_DirectOP_QueryRecord *x = reinterpret_cast<mDNS_DirectOP_QueryRecord*>(op);
    ContextExtension* context_ext = reinterpret_cast<ContextExtension*>(x->context);
    if (x->q.ThisQInterval >= 0) mDNS_StopQuery(context_ext->mDNSStorage, &x->q);
    delete x;
    delete context_ext;
}

static void DNSServiceQueryRecordResponse(mDNS *const m, DNSQuestion *question, const ResourceRecord *const answer, QC_result AddRecord) {
    mDNS_DirectOP_QueryRecord *x = reinterpret_cast<mDNS_DirectOP_QueryRecord*>(question->QuestionContext);
    char fullname[MAX_ESCAPED_DOMAIN_NAME];
    (void)m;    // Unused
    ConvertDomainNameToCString(answer->name, fullname);
    ContextExtension* context_ext = reinterpret_cast<ContextExtension*>(x->context);
    x->callback((DNSServiceRef)x, AddRecord ? kDNSServiceFlagsAdd : kDNSServiceFlagsPrivateOne, 0, kDNSServiceErr_NoError,
                fullname, answer->rrtype, answer->rrclass, answer->rdlength, answer->rdata->u.data, answer->rroriginalttl, context_ext->context);
}

DNSServiceErrorType MDNSResponder::query_record(DNSServiceRef *sdRef, DNSServiceFlags flags, uint32_t interfaceIndex,
                                                const char *fullname, uint16_t rrtype, uint16_t rrclass,
                                                DNSServiceQueryRecordReply callback, void *context) {
    mStatus err = mStatus_NoError;
    mDNS_DirectOP_QueryRecord *x;

    (void)flags;            // Unused
    (void)interfaceIndex;   // Unused

    // Allocate memory, and handle failure
    x = new mDNS_DirectOP_QueryRecord{};
    if (!x) { return mStatus_NoMemoryErr; }

    ContextExtension* context_ext = new ContextExtension;
    context_ext->context = context;
    context_ext->mDNSStorage = &_mDNS_storage;

    // Set up object
    x->disposefn = DNSServiceQueryRecordDispose;
    x->callback  = callback;
    x->context   = context_ext;

    x->q.ThisQInterval       = -1;      // So that DNSServiceResolveDispose() knows whether to cancel this question
    x->q.InterfaceID         = mDNSInterface_Any;
    x->q.flags               = flags;
    x->q.Target              = zeroAddr;
    MakeDomainNameFromDNSNameString(&x->q.qname, fullname);
    x->q.qtype               = rrtype;
    x->q.qclass              = rrclass;
    x->q.LongLived           = (flags & kDNSServiceFlagsLongLivedQuery) != 0;
    x->q.ExpectUnique        = mDNSfalse;
    x->q.ForceMCast          = (flags & kDNSServiceFlagsForceMulticast) != 0;
    x->q.ReturnIntermed      = (flags & kDNSServiceFlagsReturnIntermediates) != 0;
    x->q.SuppressUnusable    = (flags & kDNSServiceFlagsSuppressUnusable) != 0;
    x->q.SearchListIndex     = 0;
    x->q.AppendSearchDomains = 0;
    x->q.RetryWithSearchDomains = mDNSfalse;
    x->q.TimeoutQuestion     = 0;
    x->q.WakeOnResolve       = 0;
    x->q.UseBackgroundTrafficClass = (flags & kDNSServiceFlagsBackgroundTrafficClass) != 0;
    x->q.ValidationRequired  = 0;
    x->q.ValidatingResponse  = 0;
    x->q.ProxyQuestion       = 0;
    x->q.qnameOrig           = mDNSNULL;
    x->q.AnonInfo            = mDNSNULL;
    x->q.pid                 = mDNSPlatformGetPID();
    x->q.QuestionCallback    = DNSServiceQueryRecordResponse;
    x->q.QuestionContext     = x;

    err = mDNS_StartQuery(&_mDNS_storage, &x->q);
    if (err) { DNSServiceResolveDispose(reinterpret_cast<mDNS_DirectOP*>(x)); return err; }

    // Succeeded: Wrap up and return
    *sdRef = (DNSServiceRef)x;
    return(mStatus_NoError);
}

void MDNSResponder::deallocate_service_ref(DNSServiceRef sdRef) {
    mDNS_DirectOP *op = reinterpret_cast<mDNS_DirectOP*>(sdRef);
    op->disposefn(op);
}