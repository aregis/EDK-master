/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <huestream/stream/DtlsUdpClient.h>
#include <mbedtls/platform.h>
#include <mbedtls/ssl.h>
#include <mbedtls/net_sockets.h>

#include <assert.h>

#include <memory>
#include <string>

namespace huestream {

void DtlsUdpClient::Init() {
    socketUdp_ = std::make_shared<support::SocketUdp>(support::SocketAddress("0.0.0.0", 0));
}

void DtlsUdpClient::Free() {
    socketUdp_->close();
}

netstatus_e DtlsUdpClient::Connect(const std::string& host, unsigned int port) {
    remote_.set_ip(host);
    remote_.set_port(port);
    connected = true;
    if (socketUdp_->bind() != support::SOCKET_STATUS_OK)
        return MBEDTLS_ERR_NET_BIND_FAILED;
    if (socketUdp_->connect_sync(remote_) != support::SOCKET_STATUS_OK)
        return MBEDTLS_ERR_NET_CONNECT_FAILED;

    return 0;
}

int DtlsUdpClient::Send(const unsigned char *buf, size_t len) {
    assert(connected);
    return static_cast<int>(socketUdp_->sync_send(buf, len));
}

int DtlsUdpClient::ReceiveWithTimeout(unsigned char *buf, size_t len, uint32_t timeout) {
    auto retcode = socketUdp_->sync_receive(buf, len, timeout);
    switch (retcode) {
        case support::SOCKET_STATUS_AGAIN:return MBEDTLS_ERR_SSL_WANT_READ;
        case support::SOCKET_STATUS_TIMEOUT:return MBEDTLS_ERR_SSL_TIMEOUT;
        default:return static_cast<int>(retcode);
    }
}
}  // namespace huestream
