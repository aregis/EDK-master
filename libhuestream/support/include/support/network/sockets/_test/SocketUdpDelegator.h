/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <memory>
#include <string>

#include "support/network/sockets/SocketUdp.h"
#include "support/network/sockets/SocketAddress.h"
#include "support/util/StringUtil.h"

using std::shared_ptr;
using std::string;

namespace support {

    class SocketUdpDelegatorProvider {
    public:
        /**
         Get socket delegate
         @param address_local The local address to initialize the socket
         @return The socket delegate
         */
        virtual shared_ptr<SocketUdp> get_delegate(const SocketAddress& address_local) = 0;

        virtual ~SocketUdpDelegatorProvider() = default;
    };
    
    // Default
    class SocketUdpDelegatorProviderImpl : public SocketUdpDelegatorProvider {
    public:
        /**
         @see SocketUdpDelegateProvider
         */
        shared_ptr<SocketUdp> get_delegate(const SocketAddress& address_local) {
            // Create instance
            return shared_ptr<SocketUdp>(new SocketUdp(address_local));
        }
    };

    class SocketUdpDelegator : public SocketUdp {
    public:
        /** 
         @see SocketUdp.h
         */
        SocketUdpDelegator();
        
        /** 
         @see SocketUdp.h
         */
        explicit SocketUdpDelegator(const SocketAddress& address_local);
        
        /** 
         @see SocketUdp.h
         */
        ~SocketUdpDelegator() = default;
        
        /** 
         @see SocketUdp.h
         */
        int bind() override;
        
        /**
         @see SocketUdp.h
         */
        int connect_sync(const SocketAddress& address_remote) override;
        
        /**
         @see SocketUdp.h
         */
        void close() override;
        
        /**
         @see SocketUdp.h
         */
        bool is_opened() override;
        
        /**
         @see SocketUdp.h
         */
        bool is_bounded() override;
        
        /**
         @see SocketUdp.h
         */
        int async_send(const string& data, SocketCallback callback) override;
        
        /**
         @see SocketUdp.h
         */
        int async_send(const string& data, const SocketAddress& address_remote, SocketCallback callback) override;

        /** 
         @see SocketUdp.h
         */
        int async_receive(string& data, size_t buffer_size, SocketCallback callback) override;  // NOLINT
        
        /**
         @see SocketUdp.h
         */
        int async_receive(string& data, size_t buffer_size, const SocketAddress& address_remote, SocketCallback callback) override;  // NOLINT
        
        /** 
         @see SocketUdp.h
         */
        int get_reuse_address() const override;
        
        /** 
         @see SocketUdp.h
         */
        int set_reuse_address(int reuse_address) override;
        
        /** 
         @see SocketUdp.h
         */
        int get_broadcast() const override;
        
        /** 
         @see SocketUdp.h
         */
        int set_broadcast(int broadcast) override;
        
        /** 
         @see SocketUdp.h
         */
        int join_group(const SocketAddress& address_multicast) override;
        
        /** 
         @see SocketUdp.h
         */
        int leave_group(const SocketAddress& address_multicast) override;
        
        
        /* delegate provider */
        
        /**
         Set the delegate
         @note   Initially SocketUdpDelegatorProviderImpl is set as delegate
         @return The delegate, nullptr if no delegate has been set
         */
        static void set_delegate_provider(shared_ptr<SocketUdpDelegatorProvider> delegate_provider);
        
    private:
        /** the socket delegate */
        shared_ptr<SocketUdp> _delegate;
    };
    
}  // namespace support
