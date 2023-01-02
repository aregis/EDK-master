/*******************************************************************************
Copyright © 2016 Philips Lighting Holding B.V.
All Rights Reserved.
********************************************************************************/

#ifndef EDTLS_WRAPPER_MBEDTLS_MBEDTLSCLIENTWRAPPER_H_
#define EDTLS_WRAPPER_MBEDTLS_MBEDTLSCLIENTWRAPPER_H_

#include <mbedtls/build_info.h>

#if defined(MBEDTLS_PLATFORM_C)
#include <mbedtls/platform.h>
#else
#include <stdio.h>
#define mbedtls_printf     printf
#define mbedtls_fprintf    fprintf
#define mbedtls_time_t     time_t
#endif

#include <string.h>
#include <edtls/wrapper/mbedtls/UDPClientBase.h>

#include <mbedtls/debug.h>
#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/error.h>
#include <mbedtls/timing.h>
#include <mbedtls/ssl_ciphersuites.h>

#include <edtls/wrapper/IClientWrapper.h>
#include <edtls/wrapper/IClientConnectionHandler.h>
#include <edtls/wrapper/PSKInfo.h>
#include <edtls/wrapper/mbedtls/EntropyProviderBase.h>
#include <edtls/wrapper/mbedtls/MbedtlsClientPlatform.h>

class MbedtlsClientWrapper: public IClientWrapper {
 private:
  static int entropy_poll(void *data, unsigned char *output,  size_t len, size_t *olen);
  static int timing_get_delay(void *data);
  static void timing_set_delay(void *data, uint32_t int_ms, uint32_t fin_ms);

  MbedtlsClientPlatformPtr platform_;
  mbedtls_entropy_context mbedtls_entropy_context_;
  mbedtls_ssl_context ssl_;
  mbedtls_ssl_config conf_;
  mbedtls_ctr_drbg_context ctr_drbg_;
  const int supportedCS_[1] = {MBEDTLS_TLS_PSK_WITH_AES_128_GCM_SHA256};
  const char *pers_ = "dtls_client";
  IClientConnectionHandler* connectionHandler_;

  bool setup_structure(unsigned int maxConnectTimeout, unsigned int minConnectTimeout);
  virtual bool config_PSK(const PSKInfo &pskInfo);
  virtual bool seed_random_number_generator();

 public:
  explicit MbedtlsClientWrapper(MbedtlsClientPlatformPtr platform);
  virtual ~MbedtlsClientWrapper();
  MbedtlsClientWrapper(const MbedtlsClientWrapper&) = delete;
  MbedtlsClientWrapper& operator=(const MbedtlsClientWrapper&) = delete;
  virtual bool handshake();
  virtual bool init(IClientConnectionHandler *connectionHandler, unsigned int maxConnectTimeoutMs, unsigned int minConnectTimeoutMs);
  virtual void deinit();
  virtual void close();
  virtual bool connect(const char *address, const char *port, const PSKInfo &pskInfo);
  virtual unsigned int send(char *buffer, unsigned int size_bytes);
};

#endif  // EDTLS_WRAPPER_MBEDTLS_MBEDTLSCLIENTWRAPPER_H_
