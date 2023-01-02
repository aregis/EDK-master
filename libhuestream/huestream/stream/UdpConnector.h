/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#ifndef HUESTREAM_STREAM_UDPCONNECTOR_H_
#define HUESTREAM_STREAM_UDPCONNECTOR_H_

#include <memory>
#include <string>

#include "support/network/sockets/SocketUdp.h"
#include "huestream/stream/IConnector.h"

namespace huestream {

    class UdpConnector : public IConnector {
        enum class ConnectionState {
            Disconnected,
            Connected
        };

     public:
        UdpConnector();

        virtual ~UdpConnector();

        bool Connect(BridgePtr bridge, unsigned short port) override;

        void Disconnect() override;

        bool Send(const char *buffer, unsigned int bufferSize) override;

    private:
        std::string RemovePortFromIp(std::string ip) const;

        std::shared_ptr<support::SocketUdp> _socketUdp;
        support::SocketAddress _peerAddress;
        ConnectionState _connectionState;
    };
}  // namespace huestream

#endif  // HUESTREAM_STREAM_UDPCONNECTOR_H_
