/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include "support/network/sockets/bsd/_test/CMethodDelegator.h"

#include <mutex>

using std::mutex;
using std::unique_lock;

namespace support {

    /* delegate provider */
    
    static std::shared_ptr<CMethodDelegate> _delegate = std::make_shared<CMethodDelegateDefault>();
    static std::mutex _delegate_mutex;

    void CMethodDelegator::set_delegate(shared_ptr<CMethodDelegate> delegate) {
        std::unique_lock<mutex> _delegate_lock(_delegate_mutex);
        _delegate = delegate;
    }

    /* delegator */
    SOCKET_ID CMethodDelegator::socket(int domain, int type, int protocol) {
        unique_lock<mutex> _delegate_lock(_delegate_mutex);
        return _delegate->socket(domain, type, protocol);
    }

    int CMethodDelegator::bind(SOCKET_ID handle, const struct sockaddr* addr, socklen_t addr_size) {
        unique_lock<mutex> _delegate_lock(_delegate_mutex);
        return _delegate->bind(handle, addr, addr_size);
    }

    int CMethodDelegator::close(SOCKET_ID handle) {
        unique_lock<mutex> _delegate_lock(_delegate_mutex);
        return _delegate->close(handle);
    }

    int CMethodDelegator::connect(SOCKET_ID socket, const struct sockaddr* address, socklen_t address_len) {
        unique_lock<mutex> _delegate_lock(_delegate_mutex);
        return _delegate->connect(socket, address, address_len);
    }

    int CMethodDelegator::select(SOCKET_ID handle, fd_set* read_flags, fd_set* write_flags, fd_set* error_flags, struct timeval* timeout) {
        unique_lock<mutex> _delegate_lock(_delegate_mutex);
        return _delegate->select(handle, read_flags, write_flags, error_flags, timeout);
    }

    int64_t CMethodDelegator::sendto(SOCKET_ID handle, const void* buffer, size_t buffer_size, int flags, const struct sockaddr* addr, socklen_t addr_size) {
        unique_lock<mutex> _delegate_lock(_delegate_mutex);
        return _delegate->sendto(handle, buffer, buffer_size, flags, addr, addr_size);
    }

    int64_t CMethodDelegator::send(SOCKET_ID handle, const void* buffer, size_t buffer_size, int flags) {
        unique_lock<mutex> _delegate_lock(_delegate_mutex);
        return _delegate->send(handle, buffer, buffer_size, flags);
    }

    int64_t CMethodDelegator::recv(SOCKET_ID handle, void* buffer, size_t buffer_data, int flags) {
        unique_lock<mutex> _delegate_lock(_delegate_mutex);
        return _delegate->recv(handle, buffer, buffer_data, flags);
    }

    int CMethodDelegator::getsockopt(SOCKET_ID handle, int level, int name, void* value, socklen_t* value_size) {
        unique_lock<mutex> _delegate_lock(_delegate_mutex);
        return _delegate->getsockopt(handle, level, name, value, value_size);
    }

    int CMethodDelegator::setsockopt(SOCKET_ID handle, int level, int name, const void* value, socklen_t value_size) {
        unique_lock<mutex> _delegate_lock(_delegate_mutex);
        return _delegate->setsockopt(handle, level, name, value, value_size);
    }

    int CMethodDelegator::set_nonblocking(SOCKET_ID handle) {
        unique_lock<mutex> _delegate_lock(_delegate_mutex);
        return _delegate->set_nonblocking(handle);
    }

    int CMethodDelegator::get_errno() {
        unique_lock<mutex> _delegate_lock(_delegate_mutex);
        return _delegate->get_errno();
    }

    int CMethodDelegator::inet_pton(int af, const char* src, void* dst) {
        unique_lock<mutex> _delegate_lock(_delegate_mutex);
        return _delegate->inet_pton(af, src, dst);
    }
}  // namespace support
