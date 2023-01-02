/*******************************************************************************
Copyright © 2016 Philips Lighting Holding B.V.
All Rights Reserved.
********************************************************************************/

#ifndef EDTLS_WRAPPER_IPSKPROVIDER_H_
#define EDTLS_WRAPPER_IPSKPROVIDER_H_

#include <vector>
#include <string>
#include <memory>

class IPSKProvider {
 public:
  virtual ~IPSKProvider() = default;
  virtual bool getKey(const unsigned char* identity, unsigned int identityLenBytes, std::vector<unsigned char>* keyOut) = 0;
};

typedef std::shared_ptr<IPSKProvider> PSKProviderPtr;

#endif  // EDTLS_WRAPPER_IPSKPROVIDER_H_
