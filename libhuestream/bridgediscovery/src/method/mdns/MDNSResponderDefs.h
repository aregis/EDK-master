/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

extern const char ProgramName[] = "HueSDK";

typedef struct mDNS_DirectOP_struct mDNS_DirectOP;
typedef void mDNS_DirectOP_Dispose (mDNS_DirectOP *op);
struct mDNS_DirectOP_struct {
    mDNS_DirectOP_Dispose  *disposefn;
};

typedef struct {
    mDNS_DirectOP_Dispose  *disposefn;
    DNSServiceBrowseReply callback;
    void                   *context;
    DNSQuestion q;
} mDNS_DirectOP_Browse;

typedef struct {
    mDNS_DirectOP_Dispose  *disposefn;
    DNSServiceResolveReply callback;
    void                   *context;
    const ResourceRecord   *SRV;
    const ResourceRecord   *TXT;
    DNSQuestion qSRV;
    DNSQuestion qTXT;
} mDNS_DirectOP_Resolve;

typedef struct {
    mDNS_DirectOP_Dispose      *disposefn;
    DNSServiceQueryRecordReply callback;
    void                       *context;
    DNSQuestion q;
} mDNS_DirectOP_QueryRecord;

struct ContextExtension {
    mDNS* mDNSStorage = nullptr;
    void* context = nullptr;
};