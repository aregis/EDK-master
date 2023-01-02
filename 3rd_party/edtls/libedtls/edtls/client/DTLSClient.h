/*******************************************************************************
Copyright © 2016 Philips Lighting Holding B.V.
All Rights Reserved.
********************************************************************************/

#ifndef EDTLS_CLIENT_DTLSCLIENT_H_
#define EDTLS_CLIENT_DTLSCLIENT_H_

#include <edtls/wrapper/IClientWrapper.h>
#include <edtls/client/IClientNotifier.h>
#include <edtls/client/IDTLSClient.h>

#include <memory>

enum ClientState { connected, disconnected };

class DTLSClient: public IDTLSClient {
 private:
  std::shared_ptr<IClientWrapper> wrapper_;
  IClientNotifier *clientNotifier_;
  ClientState state_;
  Logger logger_;

  unsigned int maxConnectTimeoutMs_;
  unsigned int minConnectTimeoutMs_;
  virtual void handle_peer_closed();

 public:
  DTLSClient(std::shared_ptr<IClientWrapper> wrapper,
             IClientNotifier *clientNotifier);

  DTLSClient(std::shared_ptr<IClientWrapper> wrapper,
             unsigned int maxConnectTimeoutMs,
             unsigned int minConnectTimeoutMs,
             IClientNotifier *clientNotifier);

  DTLSClient(std::shared_ptr<IClientWrapper> wrapper,
             IClientNotifier *clientNotifier,
             LogFunction logFunction);

  DTLSClient(std::shared_ptr<IClientWrapper> wrapper,
             unsigned int maxConnectTimeoutMs,
             unsigned int minConnectTimeoutMs,
             IClientNotifier *clientNotifier,
             LogFunction logFunction);
  virtual ~DTLSClient();
  DTLSClient(const DTLSClient&) = delete;
  DTLSClient& operator=(const DTLSClient&) = delete;

  bool connect(const char *address, const char *port, const PSKInfo &pskInfo);
  void close();
  unsigned int send(char *buffer, unsigned int size_bytes);
};

#endif  // EDTLS_CLIENT_DTLSCLIENT_H_
