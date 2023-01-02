/*******************************************************************************
Copyright © 2016 Philips Lighting Holding B.V.
All Rights Reserved.
********************************************************************************/

#ifndef EDTLS_WRAPPER_MBEDTLS_MBEDTLSCLIENTWRAPPERFACTORY_H_
#define EDTLS_WRAPPER_MBEDTLS_MBEDTLSCLIENTWRAPPERFACTORY_H_

#include <edtls/wrapper/IClientWrapper.h>
#include <edtls/logger/Logger.h>
#include <edtls/wrapper/mbedtls/UDPClientBase.h>
#include <edtls/wrapper/mbedtls/EntropyProviderBase.h>
#include <edtls/wrapper/mbedtls/MbedtlsClientPlatform.h>

#include <memory>

class MbedtlsClientWrapperFactory {
 public:
  static std::shared_ptr<IClientWrapper> get(MbedtlsClientPlatformPtr platform);
};

#endif  // EDTLS_WRAPPER_MBEDTLS_MBEDTLSCLIENTWRAPPERFACTORY_H_
