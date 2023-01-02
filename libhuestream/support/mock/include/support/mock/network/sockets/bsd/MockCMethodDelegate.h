/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <gmock/gmock.h>

#include "support/network/sockets/bsd/_test/CMethodDelegator.h"

using support::CMethodDelegate;

namespace support_unittests {

    class MockCMethodDelegate : public CMethodDelegate {
    public:
        MOCK_METHOD3(socket, SOCKET_ID(int domain, int type, int protocol));

        MOCK_METHOD3(bind, int(SOCKET_ID handle, const struct sockaddr* addr, socklen_t addr_size));

        MOCK_METHOD1(close, int(SOCKET_ID handle));

        MOCK_METHOD3(connect, int(SOCKET_ID socket, const struct sockaddr* address, socklen_t address_len));

        MOCK_METHOD5(select, int(SOCKET_ID handle, fd_set* read_flags, fd_set* write_flags, fd_set* error_flags, struct timeval* timeout));

        MOCK_METHOD6(sendto, int64_t(SOCKET_ID handle, const void* buffer, size_t buffer_size, int flags, const struct sockaddr* addr, socklen_t addr_size));

        MOCK_METHOD4(send, int64_t(SOCKET_ID handle, const void* buffer, size_t buffer_size, int flags));

        MOCK_METHOD4(recv, int64_t(SOCKET_ID handle, void* buffer, size_t buffer_data, int flags));

        MOCK_METHOD5(getsockopt, int(SOCKET_ID handle, int level, int name, void* value, socklen_t* value_size));

        MOCK_METHOD5(setsockopt, int(SOCKET_ID handle, int level, int name, const void* value, socklen_t value_size));

        MOCK_METHOD1(set_nonblocking, int(SOCKET_ID handle));

        MOCK_METHOD0(get_errno, int());

        MOCK_METHOD3(inet_pton, int(int af, const char* src, void* dst));
    };

}  // namespace support_unittests

