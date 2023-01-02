/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include "support/network/sockets/Socket.h"
#include "support/network/sockets/SocketAddress.h"

using std::string;
using std::unique_ptr;

namespace support {

    class
    SocketUdp : public Socket {
    public:
        /** 
         Construct udp socket with default local address
         @see Socket.h
         */
        SocketUdp();
        
        /** 
         Construct socket with local address
         @see Socket.h
         */
        explicit SocketUdp(const SocketAddress& address_local);
        
        /** 
         Destructor, which will close the socket and will block until the socket
         has been closed
         */
        virtual ~SocketUdp();
        
        /**
         @see Socket.h
         */
        SocketUdp(const SocketUdp& request)          = delete;
        SocketUdp(SocketUdp&& request)               = delete;
        SocketUdp& operator=(const SocketUdp& other) = delete;
        SocketUdp& operator=(SocketUdp&& other)      = delete;

        /** 
         @see Socket.h
         */
        int bind() override;
        
        /**
         @see Socket.h
         */
        int connect_sync(const SocketAddress& address_remote) override;
        
        /**
         @see Socket.h
         */
        void close() override;
        
        /**
         @see Socket.h
         */
        bool is_opened() override;
        
        /**
         @see Socket.h
         */
        bool is_bounded() override;
        
        /**
         Not supported for the udp socket
         @return SOCKET_STATUS_FAILED
         */
        int async_send(const string& data, SocketCallback callback) override;
        
        /**
         @see Socket.h
         */
        int async_send(const string& data, const SocketAddress& address_remote, SocketCallback callback) override;
        /**
        @see Socket.h
        */
        int64_t sync_send(const unsigned char* data, size_t data_size) override;

        /** 
         @see Socket.h
         */
        int async_receive(string& data, size_t buffer_size, SocketCallback callback) override;  // NOLINT
        int64_t sync_receive(unsigned char* data, size_t data_size, unsigned int timeout) override;

        /**
         Not supported for the udp socket
         @return SOCKET_STATUS_FAILED
         */
        int async_receive(string& data, size_t buffer_size, const SocketAddress& address_remote, SocketCallback callback) override;  // NOLINT
        
        /** 
         @see Socket.h
         */
        int get_reuse_address() const override;
        
        /** 
         @see Socket.h
         */
        int set_reuse_address(int reuse_address) override;
        
        /** 
         Whether to enable broadcast mode
         @return true when to reuse the address, false otherwise. 
                 In case of an error: SOCKET_STATUS_FAILED
         */
        virtual int get_broadcast() const;
        
        /** 
         Set whether to enable broadcast mode
         @param broadcast true when broadcast mode should be enabled, false otherwise
         @return The result code, which represents one of the following values: 
                 - SOCKET_STATUS_OK:     option set successfully
                 - SOCKET_STATUS_FAILED: could not set the option
                                         e.g. when socket is not initialized        
        */
        virtual int set_broadcast(int broadcast);
        
        /** 
         Join group (multicasting)
         @param address_multicast The multicast address to join
         @return The result code, which represents one of the following values: 
                 - SOCKET_STATUS_OK:     option set successfully
                 - SOCKET_STATUS_FAILED: could not set the option
                                         e.g. when socket is not initialized
         */
        virtual int join_group(const SocketAddress& address_multicast);
        
        /** 
         Leave group (multicasting)
         @param address_multicast The multicast address to leave
         @return The result code, which represents one of the following values: 
                 - SOCKET_STATUS_OK:     option set successfully
                 - SOCKET_STATUS_FAILED: could not set the option
                                         e.g. when socket is not initialized
         */
        virtual int leave_group(const SocketAddress& address_multicast);
                                 
    private:
        struct Impl;
        /** private implementation */
        unique_ptr<Impl> _pimpl;
    };
}  // namespace support

