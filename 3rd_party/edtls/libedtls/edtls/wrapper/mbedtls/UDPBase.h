/*******************************************************************************
Copyright © 2016 Philips Lighting Holding B.V.
All Rights Reserved.
********************************************************************************/

#ifndef EDTLS_WRAPPER_MBEDTLS_UDPBASE_H_
#define EDTLS_WRAPPER_MBEDTLS_UDPBASE_H_

#include <stddef.h>
#include <stdint.h>

typedef int netstatus_e;

//  enum netstatus_e {
//  /**< Failed to open a socket. */
//      MBEDTLS_ERR_NET_SOCKET_FAILED  =                   -0x0042,
//  /**< The connection to the given server / port failed. */
//      MBEDTLS_ERR_NET_CONNECT_FAILED   =                 -0x0044,
//  /**< Binding of the socket failed. */
//      MBEDTLS_ERR_NET_BIND_FAILED    =                   -0x0046,
//  /**< Could not listen on the socket. */
//      MBEDTLS_ERR_NET_LISTEN_FAILED   =                  -0x0048,
//  /**< Could not accept the incoming connection. */
//      MBEDTLS_ERR_NET_ACCEPT_FAILED   =                  -0x004A,
//  /**< Reading information from the socket failed. */
//      MBEDTLS_ERR_NET_RECV_FAILED      =                 -0x004C,
//  /**< Sending information through the socket failed. */
//      MBEDTLS_ERR_NET_SEND_FAILED     =                  -0x004E,
//  /**< Connection was reset by peer. */
//      MBEDTLS_ERR_NET_CONN_RESET     =                   -0x0050,
//  /**< Failed to get an IP address for the given hostname. */
//      MBEDTLS_ERR_NET_UNKNOWN_HOST   =                   -0x0052,
//  /**< Buffer is too small to hold the data. */
//      MBEDTLS_ERR_NET_BUFFER_TOO_SMALL  =                -0x0043,
//  /**< The context is invalid, eg because it was free()ed. */
//      MBEDTLS_ERR_NET_INVALID_CONTEXT     =              -0x0045
//};

typedef int (*NonBlockingReadCallback)(void *ctx, unsigned char *buf, size_t len);
typedef int (*BlockingReadCallback)(void *ctx, unsigned char *buf, size_t len, uint32_t timeout);
typedef int (*WriteCallback)(void *ctx, const unsigned char *buf, size_t len);

class UDPBase {
private:
  static int Send(void *ctx, const unsigned char *buf, size_t len) {
    UDPBase *instance = static_cast<UDPBase *>(ctx);
    return instance->Send(buf, len);
  }

public:
  virtual ~UDPBase() {}

  BlockingReadCallback blockingReadCallback_ = nullptr;
  NonBlockingReadCallback nonBlockingReadCallback_ = nullptr;
  WriteCallback writeCallback_;

  UDPBase():writeCallback_(&UDPBase::Send) { }
  virtual void Init() = 0;
  virtual void Free() = 0;
  virtual int Send(const unsigned char *buf, size_t len) = 0;
};

template<class T>
class UDPNonBlocking: public T {
 private:
  static int Receive(void *ctx, unsigned char *buf, size_t len) {
    UDPNonBlocking *instance = static_cast<UDPNonBlocking *>(ctx);
    return instance->Receive(buf, len);
  }

 public:
  UDPNonBlocking() {
    this->nonBlockingReadCallback_ = Receive;
  }
  virtual ~UDPNonBlocking() = default;

  virtual int Receive(unsigned char *buf, size_t len) = 0;
};

template<class T>
class UDPBlocking: public T {
 private:
  static int ReceiveWithTimeout(void *ctx, unsigned char *buf, size_t len, uint32_t timeout) {
    UDPBlocking *instance = static_cast<UDPBlocking *>(ctx);
    return instance->ReceiveWithTimeout(buf, len, timeout);
  }

 public:
  UDPBlocking() {
    this->blockingReadCallback_ = ReceiveWithTimeout;
  }
  virtual ~UDPBlocking() = default;

  virtual int ReceiveWithTimeout(unsigned char *buf, size_t len, uint32_t timeout) = 0;
};

#endif  // EDTLS_WRAPPER_MBEDTLS_UDPBASE_H_
