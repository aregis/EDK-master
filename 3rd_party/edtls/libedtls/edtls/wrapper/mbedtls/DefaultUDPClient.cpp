/*******************************************************************************
Copyright © 2016 Philips Lighting Holding B.V.
All Rights Reserved.
********************************************************************************/

#define mbedtls_time_t     time_t
#include <mbedtls/net_sockets.h>
#include <edtls/wrapper/mbedtls/DefaultUDPClient.h>

void DefaultUDPClient::Init() {
  mbedtls_net_init(&socket_);
}

void DefaultUDPClient::Free() {
  mbedtls_net_free(&socket_);
}

netstatus_e DefaultUDPClient::Connect(const std::string &host, unsigned int port) {
  return mbedtls_net_connect(&socket_, host.c_str(), std::to_string(port).c_str(), MBEDTLS_NET_PROTO_UDP);
}

int DefaultUDPClient::ReceiveWithTimeout(unsigned char *buf, size_t len, uint32_t timeout) {
  return mbedtls_net_recv_timeout(&socket_, (unsigned char *) buf, len, timeout);
}
int DefaultUDPClient::Send(const unsigned char *buf, size_t len) {
  return mbedtls_net_send(&socket_, (unsigned char *) buf, len);
}