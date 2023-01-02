/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include "support/threading/Job.h"

using Task = support::JobTask;

#include "support/network/sockets/_test/SocketUdpDelegator.h"
#define SOCKET_UDP support::SocketUdpDelegator

namespace huesdk {
    class BridgeDiscoveryUpnpSendTask : public Task {
    public:
        /**
         Constructor
         @param socket pointer to socket owned by caller
         */
        explicit BridgeDiscoveryUpnpSendTask(SOCKET_UDP* socket);

        /**
         @see Job.h
         */
        void execute(CompletionHandler completion_handler) override;

        /**
         Returns task execution result
         @return true if task was executed successfully
         */
        bool is_successful() const;

    private:
        bool _successful;

        SOCKET_UDP* _socket;
    };

}  // namespace huesdk


