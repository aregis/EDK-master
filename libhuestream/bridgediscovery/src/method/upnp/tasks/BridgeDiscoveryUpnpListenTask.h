/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <string>
#include "support/threading/Job.h"

using Task = support::JobTask;

#include "support/network/sockets/_test/SocketUdpDelegator.h"
#define SOCKET_UDP support::SocketUdpDelegator

namespace huesdk {
    class BridgeDiscoveryUpnpListenTask : public Task {
    public:
        /**
         Constructor
         @param socket pointer to socket owned by caller
        */
        explicit BridgeDiscoveryUpnpListenTask(SOCKET_UDP* socket, const string &result = "");

        /**
         @see Job.h
         */
        void execute(CompletionHandler completion_handler) override;

        /**
         Gets task execution result
         @return const reference to result string
         */
        const string &get_result() const;

    private:
        SOCKET_UDP* _socket;
        std::string _result;
    };

}  // namespace huesdk


