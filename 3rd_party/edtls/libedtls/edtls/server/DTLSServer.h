/*******************************************************************************
Copyright © 2016 Philips Lighting Holding B.V.
All Rights Reserved.
********************************************************************************/

#ifndef EDTLS_SERVER_DTLSSERVER_H_
#define EDTLS_SERVER_DTLSSERVER_H_

#include <edtls/wrapper/IServerWrapper.h>
#include <edtls/server/IServerNotifier.h>
#include <edtls/server/MessageDispatcher.h>
#include <edtls/logger/Logger.h>
#include <edtls/server/IDTLSServer.h>

#include <memory>

enum State { started, stopped };

class DTLSServer: public IDTLSServer {
 private:
  IServerNotifier *serverNotifier_;
  State state_;
  std::shared_ptr<IServerWrapper> wrapper_;
  MessageDispatcher messageDispatcher_;
  SocketBase *socket_;
  SessionContextBase *session_;
  unsigned int connectionTimeoutSeconds_;
  std::chrono::steady_clock::time_point tsConnectionTimeout_;
  Logger logger_;

  virtual void handle_timeout();
  virtual void handle_peer_closed();
  virtual void handle_unrecoverable_read_error();
  void listen();
  void handshake();
  void read();
  void _stop();
  void resetConnectionTimeout();
  bool isConnectionTimeout();

  SocketBase *createSocket(const char *address, const char *port);
  void createSession();

 public:
  DTLSServer(std::shared_ptr<IServerWrapper> wrapper, IServerNotifier *server_notifier, LogFunction logFunction);
  virtual ~DTLSServer();
  DTLSServer(const DTLSServer&) = delete;
  DTLSServer& operator=(const DTLSServer&) = delete;
  void start_async(const char *address, const char *port, unsigned int readTimeoutSeconds);
  void stop();
};

#endif  // EDTLS_SERVER_DTLSSERVER_H_
