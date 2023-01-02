/*******************************************************************************
Copyright © 2016 Philips Lighting Holding B.V.
All Rights Reserved.
********************************************************************************/

#ifndef EDTLS_WRAPPER_MBEDTLS_DEFAULTENTROPYPROVIDER_H_
#define EDTLS_WRAPPER_MBEDTLS_DEFAULTENTROPYPROVIDER_H_

#include <edtls/wrapper/mbedtls/EntropyProviderBase.h>

class DefaultEntropyProvider : public EntropyProviderBase {
public:
    bool get_entropy(unsigned char *output, size_t len, size_t *olen) override;
};

#endif  // EDTLS_WRAPPER_MBEDTLS_DEFAULTENTROPYPROVIDER_H_
