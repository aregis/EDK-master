/*******************************************************************************
Copyright © 2016 Philips Lighting Holding B.V.
All Rights Reserved.
********************************************************************************/

#include <edtls/server/DTLSServer.h>

#include <memory>

DTLSServer::DTLSServer(std::shared_ptr <IServerWrapper> wrapper, IServerNotifier *serverNotifier, LogFunction logFunction)
    : wrapper_(wrapper),
      serverNotifier_(serverNotifier),
      messageDispatcher_(),
      state_(stopped),
      logger_(logFunction, "EDTLS_SRVR"),
      socket_(nullptr),
      session_(nullptr),
      connectionTimeoutSeconds_(10),
      tsConnectionTimeout_() {
#if defined(BUILD_INFO)
  auto buildInfoFormatString = "Build info: %s";
  logger_.Log(buildInfoFormatString, BUILD_INFO);
#endif
}

DTLSServer::~DTLSServer() {
  stop();
}

void DTLSServer::stop() {
  logger_.Log("Stopping...");
  messageDispatcher_.execBlocked([=] {
    _stop();
  });
}

void DTLSServer::_stop() {
  if (state_ == stopped) {
    logger_.Log("Skipping stop: state --> already stopped");
    return;
  }

  wrapper_->closeSession(session_);
  wrapper_->resetSession(session_);
  wrapper_->freeSession(session_);
  wrapper_->freeSocket(socket_);
  wrapper_->deinit();
  messageDispatcher_.exec([=] {
    serverNotifier_->server_stopped();
  });
  state_ = stopped;
  logger_.Log("Stopping ok");
}


SocketBase *DTLSServer::createSocket(const char *address, const char *port) {
  logger_.Log("Create socket...");
  SocketBase* socket = nullptr;
  if (wrapper_->init()) {
    socket = wrapper_->createSocket();
    if (socket) {
      if (!wrapper_->bind(socket, address, port)) {
        wrapper_->freeSocket(socket);
        logger_.Log("Create socket failed!");
        return nullptr;
      }
    }
  }
  logger_.Log("Create socket ok");
  return socket;
}

void DTLSServer::createSession() {
  logger_.Log("Create session...");
  session_ = wrapper_->createSession(*this, 1000);
  logger_.Log("Create session ok");
}

void DTLSServer::listen() {
  if (state_ != started) {
    logger_.Log("Server not started. skip listen");
    return;
  }

  if (isConnectionTimeout()) {
    logger_.Log("Connection timeout. Stopping...");
    messageDispatcher_.exec([=] { stop(); });
  } else {
    const unsigned int listenTimeMs = 100;
    if (wrapper_->listen(socket_, session_, listenTimeMs)) {
      logger_.Log("Listening ok");
      messageDispatcher_.exec([=] { handshake(); });
    } else {
      messageDispatcher_.exec([=] { listen(); });
    }
  }
}

void DTLSServer::handshake() {
  if (state_ != started) {
    logger_.Log("Server not started. skip handshake");
    return;
  }

  if (isConnectionTimeout()) {
    logger_.Log("Connection timeout. Stopping...");
    messageDispatcher_.exec([=] { stop(); });
  }

  logger_.Log("Handshaking...");
  if (wrapper_->handshake(session_)) {
    logger_.Log("Handshaking ok");
    messageDispatcher_.exec([=] { read(); });
  } else {
    logger_.Log("Handshake failed or timed-out.");
    logger_.Log("Resetting session...");
    wrapper_->resetSession(session_);
    logger_.Log("Resetting session ok");
    messageDispatcher_.exec([=] { listen(); });
  }
}

void DTLSServer::resetConnectionTimeout() {
  tsConnectionTimeout_ = std::chrono::steady_clock::now() + std::chrono::seconds(connectionTimeoutSeconds_);
}

bool DTLSServer::isConnectionTimeout() {
  return (std::chrono::steady_clock::now() > tsConnectionTimeout_);
}

void DTLSServer::read() {
  if (state_ != started) {
    return;
  }

  char buffer[1024];
  auto bytes_received = wrapper_->receive(session_, buffer, 1024);

  if (bytes_received) {
    serverNotifier_->receive_data(buffer, bytes_received);
    resetConnectionTimeout();
  }

  if (isConnectionTimeout()) {
    logger_.Log("Connection timeout. Stopping...");
    messageDispatcher_.exec([=] { stop(); });
  } else {
    messageDispatcher_.exec([=] { read(); });
  }
}

void DTLSServer::start_async(const char *address, const char *port, unsigned int readTimeoutSeconds) {
  if (state_ == started) {
    logger_.Log("Server already started. skip start server");
    return;
  }

  logger_.Log("Starting server...");

  socket_ = createSocket(address, port);
  createSession();
  connectionTimeoutSeconds_ = readTimeoutSeconds;
  state_ = started;
  resetConnectionTimeout();
  messageDispatcher_.exec([=] { listen(); });
}

void DTLSServer::handle_timeout() {
}

void DTLSServer::handle_peer_closed() {
  logger_.Log("Handle peer closed...");

  messageDispatcher_.exec([=] {
    stop();
    messageDispatcher_.exec([=] {
      logger_.Log("Sending peer closed to application");
      serverNotifier_->peer_closed();
      logger_.Log("Handle peer closed ok");
    });
  });
}

void DTLSServer::handle_unrecoverable_read_error() {
  logger_.Log("Handle unrecoverable read error...");

  messageDispatcher_.exec([=] {
    stop();
    logger_.Log("Handle unrecoverable read done");
  });
}
