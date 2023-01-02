/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <string>
#include <utility>

#include "method/upnp/tasks/BridgeDiscoveryUpnpListenTask.h"

#include "support/logging/Log.h"

#include "bridgediscovery/BridgeDiscoveryConst.h"

#include "BridgeDiscoveryUpnpListenTask.h"

using support::JobTask;
using support::SocketError;
using support::SocketStatusCode;

namespace huesdk {
    BridgeDiscoveryUpnpListenTask::BridgeDiscoveryUpnpListenTask(SOCKET_UDP* socket, const string &result)
            : _socket(socket), _result(result) {
    }

    void BridgeDiscoveryUpnpListenTask::execute(JobTask::CompletionHandler done) {
        auto socket_status = _socket->async_receive(_result, bridge_discovery_const::UDP_SOCKET_BUFFER_SIZE, [this, done](const SocketError &error, unsigned int bytes_received) {
            if (error.get_code() == support::SOCKET_STATUS_OK) {
                if (!_result.empty()) {
                    HUE_LOG << HUE_NETWORK << HUE_DEBUG << "BridgeDiscoveryUpnp: received response: " << _result
                            << ", bytes:" << bytes_received << HUE_ENDL;
                }
            }

            done();
        });

        if (socket_status != SocketStatusCode::SOCKET_STATUS_OK) {
            done();
        }
    }

    const string &BridgeDiscoveryUpnpListenTask::get_result() const {
        return _result;
    }
}  // namespace huesdk
