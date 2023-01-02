/*******************************************************************************
Copyright © 2016 Philips Lighting Holding B.V.
All Rights Reserved.
********************************************************************************/

#ifndef EDTLS_WRAPPER_MBEDTLS_ENTROPYPROVIDERBASE_H_
#define EDTLS_WRAPPER_MBEDTLS_ENTROPYPROVIDERBASE_H_

#include <cstdio>
#include <memory>

class EntropyProviderBase {
 public:
  EntropyProviderBase() {}
  virtual ~EntropyProviderBase() {}
  virtual bool get_entropy(unsigned char *output, size_t len, size_t *olen) = 0;
};

typedef std::shared_ptr<EntropyProviderBase> EntropyProviderBasePtr;

#endif  // EDTLS_WRAPPER_MBEDTLS_ENTROPYPROVIDERBASE_H_
