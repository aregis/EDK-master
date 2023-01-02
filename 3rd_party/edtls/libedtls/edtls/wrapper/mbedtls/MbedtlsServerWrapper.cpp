/*******************************************************************************
Copyright © 2016 Philips Lighting Holding B.V.
All Rights Reserved.
********************************************************************************/

#include <edtls/wrapper/mbedtls/MbedtlsServerWrapper.h>

#include <functional>
#include <sstream>
#include <chrono>
#include <vector>
#include <string>

int psk_callback(void *param,
                 mbedtls_ssl_context *ssl,
                 const unsigned char *psk_identity,
                 size_t psk_identity_len) {
  auto callbackContainter = reinterpret_cast<PskCallbackContainer *>(param);
  auto pskProvider = callbackContainter->pskProvider_;
  auto logger = callbackContainter->logger_;

  std::vector<unsigned char> key;
  auto psk_identity_string = std::string(reinterpret_cast<const char *>(psk_identity), psk_identity_len);

  //  Do not log keys, they may end up in logfiles, doing so is a security risk
  if (pskProvider->getKey(psk_identity, psk_identity_len, &key)) {
    logger.Log("pskProvider key found for identity: '%s')", psk_identity_string.c_str());
    return (mbedtls_ssl_set_hs_psk(ssl, &key.front(), key.size()));
  } else {
    logger.Log("pskProvider key NOT found for identity: '%s'", psk_identity_string.c_str());
  }

  return (-1);
}

static void my_debug(void *arg, int level,
                     const char *file, int line,
                     const char *str) {
  ((void) level);

  void (*logFunc)(const char *) = (void (*)(const char *)) arg;

  std::stringstream ss;
  ss << file << ":" << line << ": " << str;
  logFunc(ss.str().c_str());
}

MbedtlsServerWrapper::MbedtlsServerWrapper(MbedtlsServerPlatformPtr platform) :
    logger_(platform->get_log_function(), "EDTLS_SRVR_WRPR"),
    pskProvider_(platform->get_psk_provider()),
    ctr_drbg_(),
    entropy_(),
    platform_(platform),
    pskCallbackContainer_(logger_, pskProvider_) {
#if defined(MBEDTLS_DEBUG_C)
  mbedtls_debug_set_threshold(DEBUG_LEVEL);
#endif
}

void MbedtlsServerWrapper::LogMbedtlsError(const std::string &message, int errorValue) {
  char error_buf[100];
  logger_.Log("%s returned -0x%x", message.c_str(), -errorValue);
  mbedtls_strerror(errorValue, error_buf, 100);
  logger_.Log("%s => Last error was: %d - %s", message.c_str(), errorValue, error_buf);
}

SessionContextBase* MbedtlsServerWrapper::createSession(const ISessionHandler &sessionHandler,
                                                        unsigned int readTimeoutMs) {
  MbedtlsSessionContext *sessionContext = new MbedtlsSessionContext();
  sessionContext->session_handler_ = const_cast<ISessionHandler*>(&sessionHandler);
  mbedtls_net_init(&sessionContext->client_fd_);
  mbedtls_ssl_init(&sessionContext->ssl_);
  mbedtls_ssl_config_init(&sessionContext->conf_);
  mbedtls_ssl_cookie_init(&sessionContext->cookie_ctx_);
#if defined(MBEDTLS_SSL_CACHE_C)
  mbedtls_ssl_cache_init(&sessionContext->cache_);
#endif


  mbedtls_ssl_conf_read_timeout(&sessionContext->conf_, readTimeoutMs);
  mbedtls_ssl_conf_ciphersuites(&sessionContext->conf_, sessionContext->supportedCS_);
  mbedtls_ssl_conf_psk_cb(&sessionContext->conf_, psk_callback, reinterpret_cast<void *>(&pskCallbackContainer_));

  if (!setup_session(sessionContext)) {
    freeSession(sessionContext);
    sessionContext = nullptr;
  }

  return sessionContext;
}

void MbedtlsServerWrapper::freeSession(SessionContextBase *session) {
  MbedtlsSessionContext *embedtlsSessionContext = static_cast<MbedtlsSessionContext *>(session);

  if (embedtlsSessionContext) {
    logger_.Log("Deinitializing the lib...");
    mbedtls_ssl_free(&embedtlsSessionContext->ssl_);
    mbedtls_ssl_config_free(&embedtlsSessionContext->conf_);
    mbedtls_ssl_cookie_free(&embedtlsSessionContext->cookie_ctx_);
#if defined(MBEDTLS_SSL_CACHE_C)
    mbedtls_ssl_cache_free(&embedtlsSessionContext->cache_);
#endif
    logger_.Log("Deinitializing the lib done");

    delete embedtlsSessionContext;
  }
}

bool MbedtlsServerWrapper::init() {
  mbedtls_ctr_drbg_init(&ctr_drbg_);
  mbedtls_entropy_init(&entropy_);
  if (!seed_random_number_generator()) return false;
  return true;
}

void MbedtlsServerWrapper::deinit() {
  mbedtls_ctr_drbg_free(&ctr_drbg_);
  mbedtls_entropy_free(&entropy_);
}

SocketBase *MbedtlsServerWrapper::createSocket() {
  MbedtlsServerSocket *socket = new MbedtlsServerSocket();
  mbedtls_net_init(&socket->listen_fd_);

  return socket;
}

void MbedtlsServerWrapper::freeSocket(SocketBase *socket) {
  MbedtlsServerSocket *listening_socket = static_cast<MbedtlsServerSocket *>(socket);

  if (listening_socket) {
    mbedtls_net_free(&listening_socket->listen_fd_);
    delete listening_socket;
  }
}

bool MbedtlsServerWrapper::bind(SocketBase* socket, const char *address, const char *port) {
  MbedtlsServerSocket *listening_socket = static_cast<MbedtlsServerSocket *>(socket);

  if (listening_socket) {
    int ret;

    logger_.Log("Bind on udp %s/%s ...", address, port);
    if ((ret = mbedtls_net_bind(&listening_socket->listen_fd_, address, port, MBEDTLS_NET_PROTO_UDP)) != 0) {
      LogMbedtlsError("Bind failed! mbedtls_net_bind", ret);
      return false;
    }

    logger_.Log("Bind ok");
    return true;
  }
  return false;
}

bool MbedtlsServerWrapper::listen(SocketBase* socket, SessionContextBase* session, unsigned int timeoutMs) {
  auto embedtlsServerSocket = static_cast<MbedtlsServerSocket*>(socket);
  if (embedtlsServerSocket == nullptr) {
    return false;
  }

  auto embedtlsSessionContext = static_cast<MbedtlsSessionContext*>(session);
  if (embedtlsSessionContext == nullptr) {
    return false;
  }

  int ret;
  unsigned char client_ip[16] = {0};
  size_t cliip_len;
  auto tsReadTimeout = std::chrono::steady_clock::now()+ std::chrono::milliseconds(timeoutMs);
  /*
   * 3. Wait until a client connects
   */

  if ((ret = mbedtls_net_set_nonblock(&embedtlsServerSocket->listen_fd_)) != 0) {
    LogMbedtlsError("Listening failed! mbedtls_net_set_nonblock", ret);
    return false;
  }

  do {
    ret = mbedtls_net_accept(&embedtlsServerSocket->listen_fd_,
                             &embedtlsSessionContext->client_fd_,
                             client_ip, sizeof(client_ip), &cliip_len);
    if (std::chrono::steady_clock::now() > tsReadTimeout) {
      return false;
    }
  } while (ret == MBEDTLS_ERR_SSL_WANT_READ);

  if (ret < 0) {
    LogMbedtlsError("Listening failed! mbedtls_net_accept", ret);
    return false;
  }

  logger_.Log("Listening mbedtls_net_accept done");

  /* For HelloVerifyRequest cookies */
  if ((ret = mbedtls_ssl_set_client_transport_id(&embedtlsSessionContext->ssl_,
                                                 client_ip, cliip_len)) != 0) {
    LogMbedtlsError("Listening failed! 'mbedtls_ssl_set_client_transport_id()'", ret);
    return false;
  }

  mbedtls_ssl_set_bio(&embedtlsSessionContext->ssl_, &embedtlsSessionContext->client_fd_,
                      mbedtls_net_send, mbedtls_net_recv, mbedtls_net_recv_timeout);

  return true;
}

bool MbedtlsServerWrapper::handshake(SessionContextBase* session) {
  auto embedtlsSessionContext = static_cast<MbedtlsSessionContext*>(session);

  if (embedtlsSessionContext == nullptr) {
    return false;
  }

  int ret;
  logger_.Log("Performing the DTLS handshake...");

  do {
    ret = mbedtls_ssl_handshake(&embedtlsSessionContext->ssl_);
  } while (ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE);

  if (ret == MBEDTLS_ERR_SSL_HELLO_VERIFY_REQUIRED) {
    logger_.Log("Handshake not completed: Hello verification requested. Please retry...");
    return false;
  } else if (ret != 0) {
    LogMbedtlsError("Handshake failed! mbedtls_ssl_handshake", ret);
    return false;
  }

  logger_.Log("Handshake ok");
  return true;
}

unsigned int MbedtlsServerWrapper::receive(SessionContextBase* session, char *buffer, unsigned int max_bytes) {
  auto embedtlsSessionContext = static_cast<MbedtlsSessionContext*>(session);

  if (embedtlsSessionContext == nullptr) {
    return 0;
  }

  int ret;
  memset(buffer, 0, max_bytes);

  do {
    ret = mbedtls_ssl_read(&embedtlsSessionContext->ssl_, reinterpret_cast<unsigned char*>(buffer), max_bytes);
  } while (ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE);

  if (ret <= 0) {
    switch (ret) {
      case MBEDTLS_ERR_SSL_TIMEOUT:
        if (embedtlsSessionContext->session_handler_) {
          embedtlsSessionContext->session_handler_->handle_timeout();
        }
        break;
      case MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY:
        if (embedtlsSessionContext->session_handler_) {
          embedtlsSessionContext->session_handler_->handle_peer_closed();
        }
        break;
      default:LogMbedtlsError("Read mbedtls_ssl_read", ret);
        if (embedtlsSessionContext->session_handler_) embedtlsSessionContext->session_handler_->handle_unrecoverable_read_error();
    }

    ret = 0;
  }

  return static_cast<unsigned int>(ret);
}

int MbedtlsServerWrapper::entropy_poll(void *data, unsigned char *output, size_t len, size_t *olen) {
  auto provider = static_cast<EntropyProviderBase *>(data);
  return provider->get_entropy(output, len, olen) ? 0 : MBEDTLS_ERR_ENTROPY_SOURCE_FAILED;
}

bool MbedtlsServerWrapper::seed_random_number_generator() {
  int ret;
  logger_.Log("Seeding the random number generator...");

  if (platform_->get_entropy_provider()) {
    ret = mbedtls_entropy_add_source(&entropy_, entropy_poll,
                                     platform_->get_entropy_provider().get(), 32,
                                     MBEDTLS_ENTROPY_SOURCE_STRONG);
    if (ret != 0) {
      mbedtls_printf("mbedtls_entropy_add_source returned -0x%04x\n", -ret);
      return false;
    }
  }

  if ((ret = mbedtls_ctr_drbg_seed(&ctr_drbg_, mbedtls_entropy_func, &entropy_,
                                   reinterpret_cast<const unsigned char *>(pers_),
                                   strlen(pers_))) != 0) {
    LogMbedtlsError("Seeding failed! mbedtls_ctr_drbg_seed", ret);
    return false;
  }

  logger_.Log("Seeding ok");
  return true;
}

void MbedtlsServerWrapper::resetSession(SessionContextBase* session) {
  auto embedtlsSessionContext = static_cast<MbedtlsSessionContext*>(session);

  if (embedtlsSessionContext == nullptr) {
    return;
  }

  logger_.Log("Resetting the session...");
  mbedtls_net_free(&embedtlsSessionContext->client_fd_);
  mbedtls_ssl_session_reset(&embedtlsSessionContext->ssl_);
  logger_.Log("Resetting the session done");
}

bool MbedtlsServerWrapper::closeSession(SessionContextBase* session) {
  auto embedtlsSessionContext = static_cast<MbedtlsSessionContext*>(session);

  if (embedtlsSessionContext == nullptr) {
    return false;
  }

  int ret;
  logger_.Log("Closing the session...");

  /* No error checking, the connection might be closed already */
  do {
    ret = mbedtls_ssl_close_notify(&embedtlsSessionContext->ssl_);
  } while (ret == MBEDTLS_ERR_SSL_WANT_WRITE);

  logger_.Log("Closing session done");
  return true;
}

bool MbedtlsServerWrapper::setup_session(MbedtlsSessionContext *sessionContext) {
  int ret;
  logger_.Log("Setting up the DTLS data...");

  if ((ret = mbedtls_ssl_config_defaults(&sessionContext->conf_,
                                         MBEDTLS_SSL_IS_SERVER,
                                         MBEDTLS_SSL_TRANSPORT_DATAGRAM,
                                         MBEDTLS_SSL_PRESET_DEFAULT)) != 0) {
    LogMbedtlsError("Setting up the DTLS data failed! mbedtls_ssl_config_defaults", ret);
    return false;
  }

  mbedtls_ssl_conf_rng(&sessionContext->conf_, mbedtls_ctr_drbg_random, &ctr_drbg_);
  if (logger_.GetLogFunction()) {
    mbedtls_ssl_conf_dbg(&sessionContext->conf_, my_debug, reinterpret_cast<void*>(logger_.GetLogFunction()));
  }

#if defined(MBEDTLS_SSL_CACHE_C)
  mbedtls_ssl_conf_session_cache(&sessionContext->conf_, &sessionContext->cache_,
                                 mbedtls_ssl_cache_get,
                                 mbedtls_ssl_cache_set);
#endif

  if ((ret = mbedtls_ssl_cookie_setup(&sessionContext->cookie_ctx_,
                                      mbedtls_ctr_drbg_random, &ctr_drbg_)) != 0) {
    LogMbedtlsError("Setting up the DTLS failed! mbedtls_ssl_cookie_setup", ret);
    return false;
  }

  mbedtls_ssl_conf_dtls_cookies(&sessionContext->conf_, mbedtls_ssl_cookie_write, mbedtls_ssl_cookie_check,
                                &sessionContext->cookie_ctx_);

  if ((ret = mbedtls_ssl_setup(&sessionContext->ssl_, &sessionContext->conf_)) != 0) {
    LogMbedtlsError("Setting up the DTLS data failed! mbedtls_ssl_setup", ret);
    return false;
  }

  mbedtls_ssl_set_timer_cb(&sessionContext->ssl_, &sessionContext->timer_, mbedtls_timing_set_delay,
                           mbedtls_timing_get_delay);
  logger_.Log("Setting up the DTLS data ok");

  return true;
}
