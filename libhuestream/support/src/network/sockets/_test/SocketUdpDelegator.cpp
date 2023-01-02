/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include "support/network/sockets/_test/SocketUdpDelegator.h"

#include <string>
#include <mutex>

using std::mutex;
using std::unique_lock;

namespace support {
    
    /* delegate provider */
    
    static shared_ptr<SocketUdpDelegatorProvider> _delegate_provider = shared_ptr<SocketUdpDelegatorProvider>(new SocketUdpDelegatorProviderImpl());
    static mutex _delegate_provider_mutex;

    void SocketUdpDelegator::set_delegate_provider(shared_ptr<SocketUdpDelegatorProvider> delegate_provider) {
        unique_lock<mutex> _delegate_provider_lock(_delegate_provider_mutex);
        _delegate_provider = delegate_provider;
    }
    
    
    /* delegator */

    SocketUdpDelegator::SocketUdpDelegator() {
        unique_lock<mutex> _delegate_provider_lock(_delegate_provider_mutex);
        _delegate = _delegate_provider->get_delegate(SocketAddress("0.0.0.0", 0));
    }

    SocketUdpDelegator::SocketUdpDelegator(const SocketAddress& address_local) {
        unique_lock<mutex> _delegate_provider_lock(_delegate_provider_mutex);
        _delegate = _delegate_provider->get_delegate(address_local);
    }

    int SocketUdpDelegator::bind() {
        return _delegate->bind();
    }

    int SocketUdpDelegator::connect_sync(const SocketAddress& address_remote) {
        return _delegate->connect_sync(address_remote);
    }

    void SocketUdpDelegator::close() {
        return _delegate->close();
    }

    bool SocketUdpDelegator::is_opened() {
        return _delegate->is_opened();
    }

    bool SocketUdpDelegator::is_bounded() {
        return _delegate->is_bounded();
    }

    int SocketUdpDelegator::async_send(const string& data, SocketCallback callback) {
        return _delegate->async_send(data, callback);
    }

    int SocketUdpDelegator::async_send(const string& data, const SocketAddress& address_remote, SocketCallback callback) {
        return _delegate->async_send(data, address_remote, callback);
    }

    int SocketUdpDelegator::async_receive(string& data, size_t buffer_size, SocketCallback callback) {  // NOLINT
        return _delegate->async_receive(data, buffer_size, callback);
    }

    int SocketUdpDelegator::async_receive(string& data, size_t buffer_size, const SocketAddress& address_remote, SocketCallback callback) {  // NOLINT
        return _delegate->async_receive(data, buffer_size, address_remote, callback);
    }

    int SocketUdpDelegator::get_reuse_address() const {
        return _delegate->get_reuse_address();
    }

    int SocketUdpDelegator::set_reuse_address(int reuse_address) {
        return _delegate->set_reuse_address(reuse_address);
    }

    int SocketUdpDelegator::get_broadcast() const {
        return _delegate->get_broadcast();
    }

    int SocketUdpDelegator::set_broadcast(int broadcast) {
        return _delegate->set_broadcast(broadcast);
    }

    int SocketUdpDelegator::join_group(const SocketAddress& address_multicast) {
        return _delegate->join_group(address_multicast);
    }

    int SocketUdpDelegator::leave_group(const SocketAddress& address_multicast) {
        return _delegate->leave_group(address_multicast);
    }
    
}  // namespace support
