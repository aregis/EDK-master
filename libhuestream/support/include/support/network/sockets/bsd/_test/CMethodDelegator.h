/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <memory>

#include "support/network/sockets/bsd/SocketBsdCompat.h"

using std::shared_ptr;

namespace support {

    class CMethodDelegate {
    public:
        /**
         @see <sys/socket.h>
         */
        virtual SOCKET_ID socket(int domain, int type, int protocol) = 0;

        /**
         @see <sys/socket.h>
         */
        virtual int bind(SOCKET_ID handle, const struct sockaddr* addr, socklen_t addr_size) = 0;
        
        /**
         @see <unistd.h>
         */
        virtual int close(SOCKET_ID handle) = 0;

        /**
         @see <sys/socket.h>
         */
        virtual int connect(SOCKET_ID socket, const struct sockaddr* address, socklen_t address_len) = 0;

        /**
         @see <sys/_select.h>
         */
        virtual int select(SOCKET_ID handle, fd_set* read_flags, fd_set* write_flags, fd_set* error_flags, struct timeval* timeout) = 0;
        
        /**
         @see <sys/socket.h>
         */
        virtual int64_t sendto(SOCKET_ID handle, const void* buffer, size_t buffer_size, int flags, const struct sockaddr* addr, socklen_t addr_size) = 0;

        /**
        @see <sys/socket.h>
        */
        virtual int64_t send(SOCKET_ID handle, const void* buffer, size_t buffer_size, int flags) = 0;

        
        /**
         @see <sys/socket.h>
         */
        virtual int64_t recv(SOCKET_ID handle, void* buffer, size_t buffer_data, int flags) = 0;
        
        /**
         @see <sys/socket.h>
         */
        virtual int getsockopt(SOCKET_ID handle, int level, int name, void* value, socklen_t* value_size) = 0;
        
        /**
         @see <sys/socket.h>
         */
        virtual int setsockopt(SOCKET_ID handle, int level, int name, const void* value, socklen_t value_size) = 0;

        virtual int set_nonblocking(SOCKET_ID handle) = 0;

        virtual int get_errno() = 0;

        /**
         @see <arpa/inet.h>
         */
        virtual int inet_pton(int af, const char* src, void* dst) = 0;

        virtual ~CMethodDelegate() = default;
    };

#if !defined(_NX)
    class CMethodDelegateDefault : public CMethodDelegate {
    public:
        /**
         @see <sys/socket.h>
         */
        SOCKET_ID socket(int domain, int type, int protocol) override {
            return ::socket(domain, type, protocol);
        }

        /**
         @see <sys/socket.h>
         */
        int bind(SOCKET_ID handle, const struct sockaddr* addr, socklen_t addr_size) override {
            return ::bind(handle, addr, addr_size);
        }
        
        /**
         @see <unistd.h>
         */
        int close(SOCKET_ID handle) override {
            return ::closesocket(handle);
        }

        /**
         @see <sys/socket.h>
         */
        int connect(SOCKET_ID handle, const struct sockaddr* addr, socklen_t addr_size) override {
            return ::connect(handle, addr, addr_size);
        }

        /**
         @see <sys/_select.h>
         */
        int select(SOCKET_ID handle, fd_set* read_flags, fd_set* write_flags, fd_set* error_flags, struct timeval* timeout) override {
            return ::select(static_cast<int>(handle), read_flags, write_flags, error_flags, timeout);
        }

        /**
         @see <sys/socket.h>
         */
        int64_t sendto(SOCKET_ID handle, const void* buffer, size_t buffer_size, int flags, const struct sockaddr* addr, socklen_t addr_size) override {
            return ::sendto(handle, static_cast<const char*>(buffer), static_cast<int>(buffer_size), flags, addr, addr_size);
        }

        /**
        @see <sys/socket.h>
        */
        int64_t send(SOCKET_ID handle, const void* buffer, size_t buffer_size, int flags) override {
            return ::send(handle, static_cast<const char*>(buffer), static_cast<int>(buffer_size), flags);
        }

        /**
         @see <sys/socket.h>
         */
        int64_t recv(SOCKET_ID handle, void* buffer, size_t buffer_data, int flags) override {
            return ::recv(handle, static_cast<char*>(buffer), static_cast<int>(buffer_data), flags);
        }

        /**
         @see <sys/socket.h>
         */
        int getsockopt(SOCKET_ID handle, int level, int name, void* value, socklen_t* value_size) override {
            return ::getsockopt(handle, level, name, static_cast<char *>(value), value_size);
        }

        /**
         @see <sys/socket.h>
         */
        int setsockopt(SOCKET_ID handle, int level, int name, const void* value, socklen_t value_size) override {
            return ::setsockopt(handle, level, name, static_cast<const char*>(value), value_size);
        }

        int set_nonblocking(SOCKET_ID handle) override {
            return socket_set_nonblocking(handle);
        }

        int get_errno() override {
            return errno;
        }

        /**
         @see <arpa/inet.h>
         */
        int inet_pton(int af, const char* src, void* dst) override {
            return ::inet_pton(af, src, dst);
        }
    };
#else
    #define htonl       ::nn::socket::InetHtonl
    #define htons       ::nn::socket::InetHtons

    class CMethodDelegateDefault : public CMethodDelegate {
    public:
        /**
         @see <sys/socket.h>
         */
        SOCKET_ID socket(int domain, int type, int protocol) override {
            return ::nn::socket::Socket(domain, type, protocol);
        }

        /**
         @see <sys/socket.h>
         */
        int bind(SOCKET_ID handle, const struct sockaddr* addr, socklen_t addr_size) override {
            return ::nn::socket::Bind(handle, addr, addr_size);
        }

        /**
         @see <unistd.h>
         */
        int close(SOCKET_ID handle) override {
            return ::nn::socket::Close(handle);
        }

        /**
         @see <sys/socket.h>
         */
        int connect(SOCKET_ID handle, const struct sockaddr* addr, socklen_t addr_size) override {
            return ::nn::socket::Connect(handle, addr, addr_size);
        }

        /**
         @see <sys/_select.h>
         */
        int select(SOCKET_ID handle, fd_set* read_flags, fd_set* write_flags, fd_set* error_flags, struct timeval* timeout) override {
            return ::nn::socket::Select(handle, read_flags, write_flags, error_flags, timeout);
        }

        /**
         @see <sys/socket.h>
         */
        int64_t sendto(SOCKET_ID handle, const void* buffer, size_t buffer_size, int flags, const struct sockaddr* addr, socklen_t addr_size) override {
            return ::nn::socket::SendTo(handle, buffer, buffer_size, flags, addr, addr_size);
        }

        /**
        @see <sys/socket.h>
        */
        int64_t send(SOCKET_ID handle, const void* buffer, size_t buffer_size, int flags) override {
            return ::nn::socket::Send(handle, buffer, buffer_size, flags);
        }

        /**
         @see <sys/socket.h>
         */
        int64_t recv(SOCKET_ID handle, void* buffer, size_t buffer_data, int flags) override {
            return ::nn::socket::Recv(handle, buffer, buffer_data, flags);
        }

        /**
         @see <sys/socket.h>
         */
        int getsockopt(SOCKET_ID handle, int level, int name, void* value, socklen_t* value_size) override {
            return ::nn::socket::GetSockOpt(handle, level, name, value, value_size);
        }

        /**
         @see <sys/socket.h>
         */
        int setsockopt(SOCKET_ID handle, int level, int name, const void* value, socklen_t value_size) override {
            return ::nn::socket::SetSockOpt(handle, level, name, value, value_size);
        }

        int set_nonblocking(SOCKET_ID handle) override {
            return socket_set_nonblocking(handle);
        }

        int get_errno() override {
            return ::nn::socket::GetLastErrno();
        }

        /**
         @see <arpa/inet.h>
         */
        int inet_pton(int af, const char* src, void* dst) override {
            return ::nn::socket::InetPton(af, src, dst);
        }
    };
#endif
    class CMethodDelegator {
    public:
        /**
         @see <sys/socket.h>
         */
        static SOCKET_ID socket(int domain, int type, int protocol);

        /**
         @see <sys/socket.h>
         */
        static int bind(SOCKET_ID handle, const struct sockaddr* addr, socklen_t addr_size);

        /**
         @see <unistd.h>
         */
        static int close(SOCKET_ID handle);

        /**
         @see <sys/socket.h>
         */
        static int connect(SOCKET_ID socket, const struct sockaddr* address, socklen_t address_len);
        
        /**
         @see <sys/_select.h>
         */
        static int select(SOCKET_ID handle, fd_set* read_flags, fd_set* write_flags, fd_set* error_flags, struct timeval* timeout);
        
        /**
         @see <sys/socket.h>
         */
        static int64_t sendto(SOCKET_ID handle, const void* buffer, size_t buffer_size, int flags, const struct sockaddr* addr, socklen_t addr_size);

        /**
        @see <sys/socket.h>
        */
        static int64_t send(SOCKET_ID handle, const void* buffer, size_t buffer_size, int flags);

        /**
         @see <sys/socket.h>
         */
        static int64_t recv(SOCKET_ID handle, void* buffer, size_t buffer_data, int flags);
        
        /**
         @see <sys/socket.h>
         */
        static int getsockopt(SOCKET_ID handle, int level, int name, void* value, socklen_t* value_size);
        
        /**
         @see <sys/socket.h>
         */
        static int setsockopt(SOCKET_ID handle, int level, int name, const void* value, socklen_t value_size);
        
        static int set_nonblocking(SOCKET_ID handle);

        static int get_errno();

        /**
         @see <arpa/inet.h>
         */
        static int inet_pton(int af, const char* src, void* dst);

        /* delegate */
        
        /**
         Set the delegate
         @note   Initially CMethodDelegateImpl is set as delegate
         @return The delegate, nullptr if no delegate has been set
         */
        static void set_delegate(shared_ptr<CMethodDelegate> delegate);
    };

}  // namespace support
