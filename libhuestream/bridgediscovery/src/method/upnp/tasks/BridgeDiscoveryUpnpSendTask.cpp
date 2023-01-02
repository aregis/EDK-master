/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <string>
#include <utility>

#include "method/upnp/tasks/BridgeDiscoveryUpnpSendTask.h"

#include "support/network/sockets/SocketAddress.h"
#include "support/network/sockets/SocketError.h"
#include "support/logging/Log.h"

#include "bridgediscovery/BridgeDiscoveryConst.h"

#include "BridgeDiscoveryUpnpSendTask.h"

using support::JobTask;
using support::SocketAddress;
using support::SocketError;

namespace huesdk {
    BridgeDiscoveryUpnpSendTask::BridgeDiscoveryUpnpSendTask(SOCKET_UDP* socket)
            : _successful(false), _socket(std::move(socket)) {
    }

    void BridgeDiscoveryUpnpSendTask::execute(JobTask::CompletionHandler done) {
        HUE_LOG << HUE_CORE << HUE_DEBUG << "BridgeDiscoveryUpnp: sending discovery message -> ip: "
                << bridge_discovery_const::UDP_SOCKET_IP << ",port: " << bridge_discovery_const::UDP_SOCKET_PORT
                << HUE_ENDL;

        SocketAddress address_remote = SocketAddress(bridge_discovery_const::UDP_SOCKET_IP,
                                                     bridge_discovery_const::UDP_SOCKET_PORT);

        _socket->async_send(bridge_discovery_const::UPNP_REQUEST_MESSAGE, address_remote, [this, done](
                const SocketError &error, unsigned int bytes) {
            if (error.get_code() == support::SOCKET_STATUS_OK) {
                _successful = true;

                HUE_LOG << HUE_CORE << HUE_DEBUG << "BridgeDiscoveryUpnp: callback called; sending ok, bytes:"
                        << bytes << HUE_ENDL;
            } else {
                _successful = false;

                HUE_LOG << HUE_CORE << HUE_ERROR << "BridgeDiscoveryUpnp: callback called; sending failed, bytes:"
                        << bytes << HUE_ENDL;
            }

            done();
        });
    }

    bool BridgeDiscoveryUpnpSendTask::is_successful() const {
        return _successful;
    }
}  // namespace huesdk
