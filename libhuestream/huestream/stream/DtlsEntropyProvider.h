/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#ifndef HUESTREAM_STREAM_DTLSENTROPYPROVIDER_H_
#define HUESTREAM_STREAM_DTLSENTROPYPROVIDER_H_

#include "edtls/wrapper/mbedtls/EntropyProviderBase.h"

namespace huestream {

class DtlsEntropyProvider : public EntropyProviderBase {
public:
    bool get_entropy(unsigned char *output, size_t len, size_t *olen) override;
};
}  // namespace huestream

#endif  // HUESTREAM_STREAM_DTLSENTROPYPROVIDER_H_
