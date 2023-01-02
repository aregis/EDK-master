/*******************************************************************************
Copyright © 2016 Philips Lighting Holding B.V.
All Rights Reserved.
********************************************************************************/

#ifndef TEST_MOCKIPSKPROVIDER_H_
#define TEST_MOCKIPSKPROVIDER_H_

#include <vector>

#include "gmock/gmock.h"
#include "edtls/wrapper/IPSKProvider.h"

class MockIPSKProvider: public IPSKProvider {
 public:
  MOCK_METHOD3(getKey, bool(const unsigned char* identity, unsigned int identityLenBytes, std::vector<unsigned char>* keyOut));
};

#endif  // TEST_MOCKIPSKPROVIDER_H_
