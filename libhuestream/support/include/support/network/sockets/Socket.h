/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <cstdint>
#include <functional>
#include <string>

#include "support/network/sockets/SocketAddress.h"
#include "support/network/sockets/SocketError.h"
#include "support/util/StringUtil.h"

using std::function;
using std::string;
using std::size_t;

namespace support {

    enum SocketStatusCode {
        SOCKET_STATUS_OK,
        SOCKET_STATUS_FAILED  = -1,
        SOCKET_STATUS_CLOSED  = -2,
        SOCKET_STATUS_TIMEOUT = -3,
        SOCKET_STATUS_AGAIN   = -4

        // Needs to be expanded in the future more specific error messages
    };

    // Definition of the socket callback lambda
    typedef std::function<void (const SocketError& error, unsigned int bytes)> SocketCallback;
    
    class Socket {
    public:
        /**
         Construct socket with the default local address
         */
        Socket() : Socket(SocketAddress("0.0.0.0", 0)) { }
        
        /**
         Construct socket with local address
         @param address_local The local address, used for binding
                              e.g. 127.0.0.1:8080
                                   192.168.1.1:8080
         */
        explicit Socket(const SocketAddress& address_local) : _address_local(SocketAddress(address_local)) { }

        /**
         Move & copy constructors
         VS 2013 error C2610: is not a special member function which can be defaulted
         Move symantic is not implemented
         */
    public:
        Socket(const Socket& request)           = delete;
        Socket(Socket&& request)                = delete;
        Socket& operator=(const Socket& other)  = delete;
        Socket& operator=(Socket&& other)       = delete;

        /** 
         Bind the socket
         @return The result code, which represents one of the following values:
                 - SOCKET_STATUS_OK:     socket successfully bounded
                 - SOCKET_STATUS_FAILED: could not bind socket
         */
        virtual int bind() = 0;
        
        /**
         Connect to the remote address asynchronuously
         @param address_remote The remote address to connect to
         @return The result code, which represents one of the following values: 
             - SOCKET_STATUS_OK:     socket connected
             - SOCKET_STATUS_FAILED: could not connect to the remote address
         */
        virtual int connect_sync(const SocketAddress& address_remote) = 0;
        
        /** 
         Close the socket. If socket is still sending or receiving, it will cancel the operation and will
         block until the operation is cancelled
         */
        virtual void close() = 0;
        
        /**
         Whether the socket is opened
         @return true when opened, false otherwise
         */
        virtual bool is_opened() = 0;
        
        /**
         Whether the socket is bounded
         @return true when bounded, false otherwise
         */
        virtual bool is_bounded() = 0;
        
        /**
         Sending data to remote address where the socket is connected to
         @param data     The data to be sent
         @param callback The callback which will be called when sending is done
                         and provides the number of bytes sent and an error object with a message
                         and error code, which represents one of the following values:
                         - SOCKET_STATUS_OK:     data sent (partially)
                         - SOCKET_STATUS_FAILED: could not send data
         @return The result code, which represents one of the following values: 
                 - SOCKET_STATUS_OK:     sending the data
                 - SOCKET_STATUS_FAILED: could not send the data
                                         e.g. when socket is not initialized
         */
        virtual int async_send(const string& data, SocketCallback callback) = 0;
        /**
        Sending data to remote address
        @see async_send(const string& data, SocketCallback callback)
        @param address_remote The remote address where to send the data to
        */
        virtual int async_send(const string& data, const SocketAddress& address_remote, SocketCallback callback) = 0;
        
        /**
        Sending data to remote address where the socket is connected to in sync manner
        @param data         The data to be sent
        @param data_size    The size of data to be sent
        @return The result code, which represents one of the following values:
        - SOCKET_STATUS_OK:     data sent
        - SOCKET_STATUS_FAILED: could not send data
        */
        virtual int64_t sync_send(const unsigned char* data, size_t data_size) = 0;

      /**
         Receive data
         @param data     Buffer where the received data will be stored
         @param callback The callback which will be called when sending is done
                         and provides the number of bytes sent and an error object with a message
                         and an error code, which represents one of the following values:
                         - SOCKET_STATUS_OK:     data received (partially)
                         - SOCKET_STATUS_FAILED: could not send data
         @return The result code, which represents one of the following values: 
                 - SOCKET_STATUS_OK:     sending the data
                 - SOCKET_STATUS_FAILED: could not send the data
                                         e.g. when socket is not initialized
         */
        virtual int async_receive(string& data, size_t data_size, SocketCallback callback) = 0;  // NOLINT

        /**
         Receive data from remote address
         @see async_receive(string& data, size_t buffer_size, SocketCallback callback)
         @param address_remote The remote address where to receive the data from
         */
        virtual int async_receive(string& data, size_t data_size, const SocketAddress& address_remote, SocketCallback callback) = 0;  // NOLINT


        /**
         Receive data
         @param data        Buffer where the received data will be stored
         @param data_size   Size of buffer
         @param timeout     Timeout for receiving
         @return The result code, which represents one of the following values:
                 - SOCKET_STATUS_OK:     data received
                 - SOCKET_STATUS_FAILED: could not receive data
                                         e.g. when socket is not initialized
         */
        virtual int64_t sync_receive(unsigned char* data, size_t data_size, unsigned int timeout) = 0;
        
        /** 
         Whether to reuse the adddress
         @note Allows a socket to forcibly bind to a port in use by another socket
         @return true when to reuse the address, false otherwise. 
                 In case of an error: SOCKET_STATUS_FAILED
         */
        virtual int get_reuse_address() const = 0;
        
        /**
         Set whether to reuse the address
         @note Allows a socket to forcibly bind to a port in use by another socket
         @param reuse_address Whether to reuse the address:
                              - 0: disable reusing the address
                              - 1: enable reusing the address
         @return The result code, which represents one of the following values: 
                 - SOCKET_STATUS_OK:     option set successfully
                 - SOCKET_STATUS_FAILED: could not set the option
                                         e.g. when socket is not initialized
        */
        virtual int set_reuse_address(int reuse_address) = 0;
        
    protected:
        /** the local address to bind to */
        SocketAddress _address_local;
    };

}  // namespace support

