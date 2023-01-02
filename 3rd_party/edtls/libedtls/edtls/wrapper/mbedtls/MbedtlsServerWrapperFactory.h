/*******************************************************************************
Copyright © 2016 Philips Lighting Holding B.V.
All Rights Reserved.
********************************************************************************/

#ifndef EDTLS_WRAPPER_MBEDTLS_MBEDTLSSERVERWRAPPERFACTORY_H_
#define EDTLS_WRAPPER_MBEDTLS_MBEDTLSSERVERWRAPPERFACTORY_H_

#include <edtls/wrapper/IServerWrapper.h>
#include <edtls/wrapper/IPSKProvider.h>
#include <edtls/logger/Logger.h>
#include <edtls/wrapper/mbedtls/MbedtlsServerPlatform.h>

#include <memory>
#include <functional>

class MbedtlsServerWrapperFactory {
 public:
  static std::shared_ptr<IServerWrapper> get(MbedtlsServerPlatformPtr platform);
};

#endif  // EDTLS_WRAPPER_MBEDTLS_MBEDTLSSERVERWRAPPERFACTORY_H_
