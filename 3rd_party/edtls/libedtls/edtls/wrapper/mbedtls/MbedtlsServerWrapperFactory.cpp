/*******************************************************************************
Copyright © 2016 Philips Lighting Holding B.V.
All Rights Reserved.
********************************************************************************/

#include <edtls/wrapper/mbedtls/MbedtlsServerWrapperFactory.h>
#include <edtls/wrapper/mbedtls/MbedtlsServerWrapper.h>

#include <memory>

std::shared_ptr<IServerWrapper> MbedtlsServerWrapperFactory::get(MbedtlsServerPlatformPtr platform) {
  return std::make_shared<MbedtlsServerWrapper>(platform);
}
