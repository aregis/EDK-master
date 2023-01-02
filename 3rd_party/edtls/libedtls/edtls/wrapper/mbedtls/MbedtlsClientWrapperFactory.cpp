/*******************************************************************************
Copyright © 2016 Philips Lighting Holding B.V.
All Rights Reserved.
********************************************************************************/

#include <edtls/wrapper/mbedtls/MbedtlsClientWrapperFactory.h>
#include <edtls/wrapper/mbedtls/MbedtlsClientWrapper.h>

#include <memory>

std::shared_ptr<IClientWrapper> MbedtlsClientWrapperFactory::get(MbedtlsClientPlatformPtr platform) {
  return std::make_shared<MbedtlsClientWrapper>(platform);
}