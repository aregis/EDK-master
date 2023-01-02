/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#ifndef HUESTREAM_STREAM_DTLSUDPCLIENT_H_
#define HUESTREAM_STREAM_DTLSUDPCLIENT_H_

#include <string>
#include <memory>

#include "support/network/sockets/SocketUdp.h"
#include "edtls/wrapper/mbedtls/UDPBase.h"
#include "edtls/wrapper/mbedtls/UDPClientBase.h"

namespace huestream {

class DtlsUdpClient : public UDPBlocking<UDPClientBase> {
 private:
    std::shared_ptr<support::SocketUdp> socketUdp_;
    support::SocketAddress remote_;
    bool connected;

    int Send(const unsigned char *buf, size_t len) override;

 public:
    DtlsUdpClient() : socketUdp_(nullptr), remote_("", 0), connected(false) {}

    netstatus_e Connect(const std::string& host, unsigned int port) override;

    void Init() override;

    void Free() override;

    int ReceiveWithTimeout(unsigned char *buf, size_t len, uint32_t timeout) override;
};

}  // namespace huestream

#endif  // HUESTREAM_STREAM_DTLSUDPCLIENT_H_
