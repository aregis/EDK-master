/*******************************************************************************
Copyright © 2016 Philips Lighting Holding B.V.
All Rights Reserved.
********************************************************************************/

#ifndef EDTLS_WRAPPER_MBEDTLS_MBEDTLSSERVERWRAPPER_H_
#define EDTLS_WRAPPER_MBEDTLS_MBEDTLSSERVERWRAPPER_H_

#include <string>

#if !defined(MBEDTLS_CONFIG_FILE)
#include <mbedtls/config.h>
#else
#include MBEDTLS_CONFIG_FILE
#endif

#if defined(MBEDTLS_PLATFORM_C)
#include <mbedtls/platform.h>
#endif


#if defined(_WIN32)
#include <windows.h>
#endif

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <edtls/wrapper/IPSKProvider.h>

#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/certs.h>
#include <mbedtls/ssl.h>
#include <mbedtls/ssl_cookie.h>
#include <mbedtls/net.h>
#include <mbedtls/error.h>
#include <mbedtls/debug.h>
#include <mbedtls/timing.h>
#if defined(MBEDTLS_SSL_CACHE_C)
#include <mbedtls/ssl_cache.h>
#endif

#include <mbedtls/ssl_ciphersuites.h>
#include <edtls/wrapper/IServerWrapper.h>
#include <edtls/logger/Logger.h>
#include <edtls/wrapper/mbedtls/MbedtlsServerPlatform.h>

#define DEBUG_LEVEL 0

struct MbedtlsServerSocket: public SocketBase{
  MbedtlsServerSocket(): listen_fd_() {}
  mbedtls_net_context listen_fd_;
};

struct MbedtlsSessionContext: public SessionContextBase{
  MbedtlsSessionContext():
      session_handler_(nullptr),
      client_fd_(),
      cookie_ctx_(),
      ssl_(),
      conf_(),
      timer_()
#if defined(MBEDTLS_SSL_CACHE_C)
  , cache_()
#endif
{}
  MbedtlsSessionContext(const MbedtlsSessionContext&) = default;
  MbedtlsSessionContext& operator=(const MbedtlsSessionContext&) = default;
  ISessionHandler* session_handler_;
  mbedtls_net_context client_fd_;
  mbedtls_ssl_cookie_ctx cookie_ctx_;
  mbedtls_ssl_context ssl_;
  mbedtls_ssl_config conf_;
  mbedtls_timing_delay_context timer_;
#if defined(MBEDTLS_SSL_CACHE_C)
  mbedtls_ssl_cache_context cache_;
#endif
  const int supportedCS_[1] = {MBEDTLS_TLS_PSK_WITH_AES_128_GCM_SHA256};
};

struct PskCallbackContainer{
  PskCallbackContainer(const Logger& logger, PSKProviderPtr pskProvider): logger_(logger), pskProvider_(pskProvider) { }
  PskCallbackContainer(const PskCallbackContainer&) = delete;
  PskCallbackContainer& operator=(const PskCallbackContainer&) = delete;
  Logger logger_;
  PSKProviderPtr pskProvider_;
};

class MbedtlsServerWrapper: public IServerWrapper {
 private:
  Logger logger_;
  PSKProviderPtr pskProvider_;
  const char *pers_ = "dtls_server";
  mbedtls_ctr_drbg_context ctr_drbg_;
  mbedtls_entropy_context entropy_;
  MbedtlsServerPlatformPtr platform_;
  PskCallbackContainer pskCallbackContainer_;
  bool seed_random_number_generator();
  bool setup_session(MbedtlsSessionContext *sessionContext);
  static int entropy_poll(void *data, unsigned char *output, size_t len, size_t *olen);
  void LogMbedtlsError(const std::string &message, int errorValue);

 public:
  explicit MbedtlsServerWrapper(MbedtlsServerPlatformPtr platform);
  virtual ~MbedtlsServerWrapper() = default;
  MbedtlsServerWrapper(const MbedtlsServerWrapper&) = delete;
  MbedtlsServerWrapper& operator=(const MbedtlsServerWrapper&) = delete;
  virtual unsigned int receive(SessionContextBase* session, char* buffer, unsigned int max_bytes);
  virtual void deinit();
  virtual SocketBase* createSocket();
  virtual void freeSocket(SocketBase* socket);
  virtual bool bind(SocketBase* socket, const char* address, const char *port);
  virtual SessionContextBase* createSession(const ISessionHandler &sessionHandler, unsigned int readTimeoutMs);
  virtual bool listen(SocketBase* socket, SessionContextBase* session, unsigned int timeoutMs);
  virtual bool handshake(SessionContextBase* session);
  virtual void resetSession(SessionContextBase* session);
  virtual bool init();
  virtual bool closeSession(SessionContextBase* session);
  virtual void freeSession(SessionContextBase* session);
};

#endif  // EDTLS_WRAPPER_MBEDTLS_MBEDTLSSERVERWRAPPER_H_
