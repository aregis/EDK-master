/*******************************************************************************
Copyright © 2016 Philips Lighting Holding B.V.
All Rights Reserved.
********************************************************************************/

#include <edtls/client/DTLSClient.h>

#include <memory>

#define DEFAULT_MIN_CONNECT_TIMEOUT_MS (100)
#define DEFAULT_MAX_CONNECT_TIMEOUT_MS (6000)

DTLSClient::DTLSClient(std::shared_ptr<IClientWrapper> wrapper,
                       unsigned int maxConnectTimeoutMs,
                       unsigned int minConnectTimeoutMs,
                       IClientNotifier *clientNotifier,
                       LogFunction logFunction)
    : wrapper_(wrapper),
      clientNotifier_(clientNotifier),
      state_(disconnected),
      logger_(logFunction, "EDTLS_CLNT"),
      maxConnectTimeoutMs_(maxConnectTimeoutMs),
      minConnectTimeoutMs_(minConnectTimeoutMs) {
#if defined(BUILD_INFO)
  auto buildInfoFormatString = "Build info: %s";
  logger_.Log(buildInfoFormatString, BUILD_INFO);
#endif
}

static void DefaultPrintfLogger(const char* s) {
  printf("%s", s);
}

DTLSClient::DTLSClient(std::shared_ptr<IClientWrapper> wrapper,
                       IClientNotifier *clientNotifier) : DTLSClient(wrapper,
                                                                     clientNotifier,
                                                                     DefaultPrintfLogger) {}

DTLSClient::DTLSClient(std::shared_ptr<IClientWrapper> wrapper,
                       IClientNotifier *clientNotifier,
                       LogFunction logFunction) : DTLSClient(wrapper,
                                                             DEFAULT_MAX_CONNECT_TIMEOUT_MS,
                                                             DEFAULT_MIN_CONNECT_TIMEOUT_MS,
                                                             clientNotifier,
                                                             logFunction) {}

DTLSClient::DTLSClient(std::shared_ptr<IClientWrapper> wrapper,
                       unsigned int maxConnectTimeoutMs,
                       unsigned int minConnectTimeoutMs,
                       IClientNotifier *clientNotifier) : DTLSClient(wrapper,
                                                                     maxConnectTimeoutMs,
                                                                     minConnectTimeoutMs,
                                                                     clientNotifier,
                                                                     DefaultPrintfLogger) {}

DTLSClient::~DTLSClient() {
  close();
}

unsigned int DTLSClient::send(char *buffer, unsigned int size_bytes) {
  unsigned int size_written = 0;

  if (state_ == connected) {
    size_written = wrapper_->send(buffer, size_bytes);
  }

  return size_written;
}

bool DTLSClient::connect(const char *address, const char *port, const PSKInfo &pskInfo) {
  close();

  wrapper_->init(this, maxConnectTimeoutMs_, minConnectTimeoutMs_);

  if (!wrapper_->connect(address, port, pskInfo)) {
    return false;
  }

  if (wrapper_->handshake()) {
    state_ = connected;
    return true;
  } else {
    if (clientNotifier_) clientNotifier_->handshakeFailed();
  }

  return false;
}

void DTLSClient::close() {
  if (state_ == connected) {
    wrapper_->close();
    wrapper_->deinit();
    state_ = disconnected;
  }
}

void DTLSClient::handle_peer_closed() {
}
