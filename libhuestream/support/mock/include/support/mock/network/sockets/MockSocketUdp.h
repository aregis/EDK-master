/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <gmock/gmock.h>

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>

#include "support/threading/Thread.h"
#include "support/network/sockets/SocketUdp.h"
#include "support/network/sockets/SocketError.h"
#include "support/network/sockets/SocketAddress.h"
#include "support/network/sockets/Socket.h"

using std::atomic;
using std::mutex;
using std::string;
using support::SocketAddress;
using support::SocketCallback;
using support::SocketStatusCode;
using support::SocketError;
using support::SocketUdp;
using support::Thread;
using testing::Invoke;
using testing::_;

namespace support_unittests {

    class MockSocketUdp : public SocketUdp {
    public:
        /**
        
         */
        explicit MockSocketUdp(const SocketAddress& address_local) : SocketUdp(address_local),
                                                                     // By default the socket is opened successfully
                                                                     _opened(true),
                                                                     _bounded(false),
                                                                     _sending(false),
                                                                     _receiving(false) {
            // Delegate mocked calls to default implementation
            ON_CALL(*this, bind())
                .WillByDefault(Invoke(this, &MockSocketUdp::default_bind));
            ON_CALL(*this, connect(_, _))
                .WillByDefault(Invoke(this, &MockSocketUdp::default_connect));
            ON_CALL(*this, close())
                .WillByDefault(Invoke(this, &MockSocketUdp::default_close));
            ON_CALL(*this, is_opened())
                .WillByDefault(Invoke(this, &MockSocketUdp::default_is_opened));
            ON_CALL(*this, is_bounded())
                .WillByDefault(Invoke(this, &MockSocketUdp::default_is_bounded));
            ON_CALL(*this, async_send(_, _))
                .WillByDefault(Invoke(this, &MockSocketUdp::default_async_send));
            ON_CALL(*this, async_send(_, _, _))
                .WillByDefault(Invoke(this, &MockSocketUdp::default_async_send2));
            ON_CALL(*this, async_receive(_, _, _))
                .WillByDefault(Invoke(this, &MockSocketUdp::default_async_receive));
            ON_CALL(*this, async_receive(_, _, _, _))
                .WillByDefault(Invoke(this, &MockSocketUdp::default_async_receive2));
            ON_CALL(*this, get_reuse_address())
                .WillByDefault(Invoke(this, &MockSocketUdp::default_get_reuse_address));
            ON_CALL(*this, set_reuse_address(_))
                .WillByDefault(Invoke(this, &MockSocketUdp::default_set_reuse_address));
            ON_CALL(*this, get_broadcast())
                .WillByDefault(Invoke(this, &MockSocketUdp::default_get_broadcast));
            ON_CALL(*this, set_broadcast(_))
                .WillByDefault(Invoke(this, &MockSocketUdp::default_set_broadcast));
            ON_CALL(*this, join_group(_))
                .WillByDefault(Invoke(this, &MockSocketUdp::default_join_group));
            ON_CALL(*this, leave_group(_))
                .WillByDefault(Invoke(this, &MockSocketUdp::default_leave_group));
        }

        /** 
         
         */
        MOCK_METHOD0(bind, int());
        
        /**
        
         */
        MOCK_METHOD2(connect, int(const SocketAddress& address_remote, SocketCallback callback));
        
        /**
         
         */
        MOCK_METHOD0(close, void());
        
        /**
        
         */
        MOCK_METHOD0(is_opened, bool());
        
        /**
        
         */
        MOCK_METHOD0(is_bounded, bool());
        
        /**
        
         */
        MOCK_METHOD2(async_send, int(const string& data, SocketCallback callback));
        
        /**
         
         */
        MOCK_METHOD3(async_send, int(const string& data, const SocketAddress& address_remote, SocketCallback callback));

        /** 
         
         */
        MOCK_METHOD3(async_receive, int(string& data, size_t buffer_size, SocketCallback callback));  // NOLINT
        
        /**
        
         */
        MOCK_METHOD4(async_receive, int(string& data, size_t buffer_size, const SocketAddress& address_remote, SocketCallback callback));  // NOLINT
        
        /** 
         
         */
        MOCK_CONST_METHOD0(get_reuse_address, int());
        
        /** 
         
         */
        MOCK_METHOD1(set_reuse_address , int(int reuse_address));
        
        /** 
         
         */
        MOCK_CONST_METHOD0(get_broadcast , int());
        
        /** 
         
         */
        MOCK_METHOD1(set_broadcast, int(int broadcast));
        
        /** 
         
         */
        MOCK_METHOD1(join_group, int(const SocketAddress& address_multicast));
        
        /** 
         
         */
        MOCK_METHOD1(leave_group, int(const SocketAddress& address_multicast));


        /* fake data */

        /**
        
         */
        MOCK_METHOD0(fake_sending_delay, unsigned int());
        
        /**
        
         */
        MOCK_METHOD0(fake_sending_error, SocketError());
        
        /**
        
         */
        MOCK_METHOD0(fake_receiving_delay, unsigned int());
                
        /**
        
         */
        MOCK_METHOD0(fake_receiving_error, SocketError());
        
        /**
        
         */
        MOCK_METHOD0(fake_receiving_data, string());

    protected:
        /** */
        atomic<bool>        _opened;
        /** */
        atomic<bool>        _bounded;
        
        /** */
        bool               _sending;
        /** */
        unique_ptr<Thread> _sending_thread;
        /** */
        bool               _receiving;
        /** */
        unique_ptr<Thread> _receiving_thread;
        
        /** */
        mutex              _socket_mutex;
        
        /** 
         
         */
        SocketStatusCode default_bind();
        
        /**
        
         */
        SocketStatusCode default_connect(const SocketAddress& address_remote, SocketCallback callback);
        
        /**
         
         */
        void default_close();
        
        /**
        
         */
        bool default_is_opened();
        
        /**
        
         */
        bool default_is_bounded();
        
        /**
         
         */
        int default_async_send(const string& data, SocketCallback callback);
        
        /**
         
         */
        int default_async_send2(const string& data, const SocketAddress& address_remote, SocketCallback callback);

        /** 
         
         */
        int default_async_receive(string& data, size_t buffer_size, SocketCallback callback);  // NOLINT
        
        /**
        
         */
        int default_async_receive2(string& data, size_t buffer_size, const SocketAddress& address_remote, SocketCallback callback);  // NOLINT
        
        /** 
         
         */
        int default_get_reuse_address() const;
        
        /** 
         
         */
        int default_set_reuse_address(int reuse_address);
        
        /** 
         
         */
        int default_get_broadcast() const;
        
        /** 
         
         */
        int default_set_broadcast(int broadcast);
        
        /** 
         
         */
        int default_join_group(const SocketAddress& address_multicast);
        
        /** 
         
         */
        int default_leave_group(const SocketAddress& address_multicast);
        
        /**
        
         */
        void thread_send(string data, SocketAddress address_remote, SocketCallback callback);
        
        /**
        
         */
        void thread_receive(string* data, size_t buffer_size, SocketCallback callback);
    };

}  // namespace support_unittests

