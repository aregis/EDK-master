/*******************************************************************************
Copyright © 2016 Philips Lighting Holding B.V.
All Rights Reserved.
********************************************************************************/

#include <edtls/wrapper/mbedtls/MbedtlsClientWrapper.h>

#include <sstream>
#include <string>

static void my_debug(void *arg, int level,
                     const char *file, int line,
                     const char *str) {
  ((void) level);

  void (*logFunc)(const char *) = (void (*)(const char *)) arg;
  std::stringstream ss;
  ss << file << ":" << line << ": " << str;
  logFunc(ss.str().c_str());
}

MbedtlsClientWrapper::MbedtlsClientWrapper(MbedtlsClientPlatformPtr platform)
    : IClientWrapper(Logger(platform->get_log_function(), "MBEDTLS_CLNT_W")),
      platform_(platform),
      mbedtls_entropy_context_(),
      ssl_(),
      conf_(),
      ctr_drbg_(),
      connectionHandler_() {
}

MbedtlsClientWrapper::~MbedtlsClientWrapper() {}

int MbedtlsClientWrapper::entropy_poll(void *data, unsigned char *output, size_t len, size_t *olen) {
  auto provider = reinterpret_cast<EntropyProviderBase*>(data);
  return provider->get_entropy(output, len, olen) ? 0: MBEDTLS_ERR_ENTROPY_SOURCE_FAILED;
}

int MbedtlsClientWrapper::timing_get_delay(void *data) {
  auto provider = reinterpret_cast<TimerProvider *>(data);
  return provider->get_delay();
}

void MbedtlsClientWrapper::timing_set_delay(void *data, uint32_t int_ms, uint32_t fin_ms) {
  auto provider = reinterpret_cast<TimerProvider *>(data);
  provider->set_delay(int_ms, fin_ms);
}

bool MbedtlsClientWrapper::init(IClientConnectionHandler *connectionHandler,
                                 unsigned int maxConnectTimeoutMs,
                                 unsigned int minConnectTimeoutMs) {
  connectionHandler_ = connectionHandler;
  platform_->get_udp_client()->Init();
  mbedtls_ssl_init(&ssl_);
  mbedtls_ssl_config_init(&conf_);
  mbedtls_ctr_drbg_init(&ctr_drbg_);

  if (!seed_random_number_generator()) {
    return false;
  }

  return setup_structure(maxConnectTimeoutMs, minConnectTimeoutMs);
}

void MbedtlsClientWrapper::deinit() {
  logger_.Log("Deinit...");
  mbedtls_ssl_free(&ssl_);
  mbedtls_ssl_config_free(&conf_);
  mbedtls_ctr_drbg_free(&ctr_drbg_);
  mbedtls_entropy_free(&mbedtls_entropy_context_);
  logger_.Log("Deinit ok");
}

bool MbedtlsClientWrapper::setup_structure(unsigned int maxConnectTimeout, unsigned int minConnectTimeout) {
  int ret;

  logger_.Log("Setting up the structure...");

  if ((ret = mbedtls_ssl_config_defaults(&conf_,
                                         MBEDTLS_SSL_IS_CLIENT,
                                         MBEDTLS_SSL_TRANSPORT_DATAGRAM,
                                         MBEDTLS_SSL_PRESET_DEFAULT)) != 0) {
    logger_.Log("Setting up the structure failed! mbedtls_ssl_config_defaults returned %d", ret);
    return false;
  }

  /* OPTIONAL is usually a bad choice for security, but makes interop easier
   * in this simplified example, in which the ca chain is hardcoded.
   * Production code should set a proper ca chain and use REQUIRED. */
//    mbedtls_ssl_conf_authmode( &conf, MBEDTLS_SSL_VERIFY_OPTIONAL );
//    mbedtls_ssl_conf_ca_chain( &conf, &cacert, NULL );
  mbedtls_ssl_conf_authmode(&conf_, MBEDTLS_SSL_VERIFY_NONE);
  mbedtls_ssl_conf_ciphersuites(&conf_, supportedCS_);
  mbedtls_ssl_conf_rng(&conf_, mbedtls_ctr_drbg_random, &ctr_drbg_);

  if (logger_.GetLogFunction()) {
    mbedtls_ssl_conf_dbg(&conf_, my_debug, reinterpret_cast<void *>(logger_.GetLogFunction()));
  }

  mbedtls_ssl_conf_handshake_timeout(&conf_, minConnectTimeout, maxConnectTimeout);
  if ((ret = mbedtls_ssl_setup(&ssl_, &conf_)) != 0) {
    logger_.Log("Setting up the structure failed! mbedtls_ssl_setup returned %d", ret);
    return false;
  }

  logger_.Log("Setting up the structure ok");

  return true;
}

bool MbedtlsClientWrapper::connect(const char *address, const char *port, const PSKInfo &pskInfo) {
  int ret;

  platform_->get_udp_client()->Init();
  mbedtls_ssl_session_reset(&ssl_);

  if (!config_PSK(pskInfo)) {
    return false;
  }

  logger_.Log("Connecting to udp %s/%s...", address, port);

  if ((ret = platform_->get_udp_client()->Connect(address, std::stoi(port))) != 0) {
    logger_.Log("Connecting failed! mbedtls_net_connect returned %d", ret);
    return false;
  }

  mbedtls_ssl_set_bio(&ssl_,
                      platform_->get_udp_client().get(),
                      platform_->get_udp_client()->writeCallback_,
                      platform_->get_udp_client()->nonBlockingReadCallback_,
                      platform_->get_udp_client()->blockingReadCallback_);

  if (!platform_->get_timer_provider()) {
    return false;
  }

  mbedtls_ssl_set_timer_cb(&ssl_,
                           platform_->get_timer_provider().get(),
                           timing_set_delay,
                           timing_get_delay);

  logger_.Log("Connecting ok");
  return true;
}

unsigned int MbedtlsClientWrapper::send(char *buffer, unsigned int size_bytes) {
  int ret;

  do {
    ret = mbedtls_ssl_write(&ssl_, reinterpret_cast<unsigned char *>(buffer), size_bytes);
  } while (ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE);

  if (ret < 0) {
    logger_.Log("Send failed! mbedtls_ssl_write returned %d", ret);
    ret = 0;
  }

  return static_cast<unsigned int>(ret);
}

bool MbedtlsClientWrapper::seed_random_number_generator() {
  int ret;
  logger_.Log("Seeding the Client random number generator...");
  mbedtls_entropy_init(&mbedtls_entropy_context_);
 
  ret = mbedtls_entropy_add_source(&mbedtls_entropy_context_, MbedtlsClientWrapper::entropy_poll,
                                   platform_->get_entropy_provider().get(), 32,
                                   MBEDTLS_ENTROPY_SOURCE_STRONG);
  if (ret != 0) {
    mbedtls_printf("mbedtls_entropy_add_source returned -0x%04x\n", -ret);
    return false;
  }
  if ((ret = mbedtls_ctr_drbg_seed(&ctr_drbg_, mbedtls_entropy_func, &mbedtls_entropy_context_,
                                   reinterpret_cast<const unsigned char *>(pers_),
                                   strlen(pers_))) != 0) {
    logger_.Log("Seeding failed! mbedtls_ctr_drbg_seed returned %d", ret);
    return false;
  }

  logger_.Log("Seeding ok");
  return true;
}

bool MbedtlsClientWrapper::config_PSK(const PSKInfo &pskInfo) {
  mbedtls_ssl_conf_psk(&conf_,
                       pskInfo.psk_.key_,
                       sizeof(pskInfo.psk_.key_),
                       reinterpret_cast<const unsigned char *>(pskInfo.identity_),
                       strlen(pskInfo.identity_));
  return true;
}

bool MbedtlsClientWrapper::handshake() {
  int ret;
  logger_.Log("Performing the SSL/TLS handshake...");

  do {
    ret = mbedtls_ssl_handshake(&ssl_);
  } while (ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE);

  if (ret != 0) {
    char error_buf[100];
    logger_.Log("Handshake failed! mbedtls_ssl_handshake returned -0x%x", -ret);
    mbedtls_strerror(ret, error_buf, 100);
    logger_.Log("Handshake failed! Last error was: %d - %s", ret, error_buf);
    return false;
  }

  logger_.Log("Handshake ok");

  return true;
}

void MbedtlsClientWrapper::close() {
  int ret;
  logger_.Log("Closing the client connection...");

  /* No error checking, the connection might be closed already */
  do {
    logger_.Log("mbedtls_ssl_close_notify...");
    ret = mbedtls_ssl_close_notify(&ssl_);
    logger_.Log("mbedtls_ssl_close_notify done");
  } while (ret == MBEDTLS_ERR_SSL_WANT_WRITE);

  platform_->get_udp_client()->Free();

  logger_.Log("Closing done\n");
}
