/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <sys/types.h>

#include <atomic>
#include <condition_variable>
#include <string>

#include "support/threading/Thread.h"
#include "support/logging/Log.h"
#include "support/network/sockets/bsd/SocketConst.h"
#include "support/network/sockets/bsd/SocketBsd.h"
#include "support/network/sockets/SocketUdp.h"
using std::atomic_bool;
using std::mutex;
using std::unique_ptr;
using std::unique_lock;
using std::string;
using support::to_string;

#ifdef _WIN32
#define SIZE_CAST static_cast<int>
#else 
#define SIZE_CAST static_cast<size_t>
#endif

namespace support {
    struct SocketUdp::Impl {
        /** Constructor */
        Impl() {
            _opened = false;
            _bounded = false;
            _sending = false;
            _receiving = false;
            _connected = false;
            _socket = 0;
            _socket_address_local = {};
        }
        /** whether the socket is opened */
        atomic_bool        _opened;
        /** whether the socket is bounded */
        atomic_bool        _bounded;
        /** whether a socket is connected (used only synchonious send and receive API) */
        atomic_bool _connected;

        /** whether the socket is sending data */
        atomic_bool               _sending;
        /** thread which sends the data */
        unique_ptr<Thread>        _sending_thread;
        /** whether the socket is receiving data */
        atomic_bool               _receiving;
        /** thread which receives the data */
        unique_ptr<Thread> _receiving_thread;

        /** the handle of the socket */
        SOCKET_ID          _socket;

        /** ensure thread safety in sending/receiving data */
        mutex              _socket_mutex;
        /** the local address of the socket, usually for binding */
        sockaddr_in        _socket_address_local;
        /**
         Thread for sending data
         */
        void thread_send_data(string data, SocketAddress address_remote, SocketCallback callback);

        /**
         Thread for receiving data
         */
        void thread_receive_data(string& data, size_t data_size, SocketCallback callback);  // NOLINT
    };

    /** initialize this socket with a default local address */
    SocketUdp::SocketUdp() : SocketUdp(SocketAddress("0.0.0.0", 0)) {
    }

    SocketUdp::SocketUdp(const SocketAddress& address_local) : Socket(SocketAddress(address_local)),
                                                               _pimpl(new Impl()) {
        SocketInitializer::Init();

        string ip         = address_local.get_ip();
        unsigned int port = address_local.get_port();


        _pimpl->_socket_address_local.sin_family      = AF_INET;
        _pimpl->_socket_address_local.sin_port        = htons(static_cast<u_short>(port));

        if (INET_PTON(AF_INET, ip.c_str(), &_pimpl->_socket_address_local.sin_addr) != 1) {
            // TODO(Wies): check error handling
            _pimpl->_socket = INVALID_SOCKET;
            return;
        }

        _pimpl->_socket = _SOCKET(AF_INET, SOCK_DGRAM, 0);

        _pimpl->_opened = _pimpl->_socket != INVALID_SOCKET;
    }

    SocketUdp::~SocketUdp() {
        // Close the socket if open
        close();
    }

    int SocketUdp::bind() {
        int status = SOCKET_STATUS_OK;

        if (_pimpl->_opened
            && !_pimpl->_bounded) {
            // Lock
            unique_lock<mutex> socket_lock(_pimpl->_socket_mutex);

            // Bind the socket to the local address
            status = BIND(_pimpl->_socket, (struct sockaddr*)&_pimpl->_socket_address_local, sizeof(_pimpl->_socket_address_local));

            if (status != SOCKET_STATUS_FAILED) {
                _pimpl->_bounded = true;
            }
        }

        return status;
    }

    int SocketUdp::connect_sync(const SocketAddress& address_remote) {
        if (!_pimpl->_opened) {
            return SOCKET_STATUS_FAILED;
        }

        if (_pimpl->_connected) {
            return SOCKET_STATUS_OK;
        }

        struct sockaddr_in socket_address_remote;
        socket_address_remote.sin_family = AF_INET;
        socket_address_remote.sin_port = htons(static_cast<u_short>(address_remote.get_port()));

        if (INET_PTON(AF_INET, address_remote.get_ip().c_str(), &socket_address_remote.sin_addr) != 1) {
            // TODO(Wies): check error handling
            return SOCKET_STATUS_FAILED;
        }

        auto stat = CONNECT(_pimpl->_socket, reinterpret_cast<const sockaddr*>(&socket_address_remote), sizeof socket_address_remote);

        _pimpl->_connected = stat >= 0;

        return _pimpl->_connected ? SOCKET_STATUS_OK : SOCKET_STATUS_FAILED;
    }

    void SocketUdp::close() {
        if (_pimpl->_opened) {
            _pimpl->_opened    = false;
            _pimpl->_bounded   = false;

            // Lock
            unique_lock<mutex> socket_lock(_pimpl->_socket_mutex);

            // Close the socket
            CLOSE(_pimpl->_socket);
        }

        if (_pimpl->_sending_thread != nullptr) {
            // Wait until the sending thread is finished
            _pimpl->_sending_thread->join();
        }
        if (_pimpl->_receiving_thread != nullptr) {
            // Wait until the receiving thread is finished
            _pimpl->_receiving_thread->join();
        }
    }

    bool SocketUdp::is_opened() {
        return _pimpl->_opened;
    }

    bool SocketUdp::is_bounded() {
        return _pimpl->_bounded;
    }

    int SocketUdp::async_send(const string& /*data*/, SocketCallback /*callback*/) {
        // Not supported
        return SOCKET_STATUS_FAILED;
    }

    int SocketUdp::async_send(const string& data, const SocketAddress& address_remote, SocketCallback callback) {
        // Check whether the socket is opened or already sending
        if (!_pimpl->_opened
            || _pimpl->_sending) {
            return SOCKET_STATUS_FAILED;
        }

        // Lock
        unique_lock<mutex> socket_lock(_pimpl->_socket_mutex);

        // Initialize sending task
        ThreadTask task = [this, data, address_remote, callback] () -> void { _pimpl->thread_send_data(data, address_remote, callback); };
        // Start thread with the task
        _pimpl->_sending_thread = unique_ptr<Thread>(new Thread(task));

        _pimpl->_sending = true;

        return SOCKET_STATUS_OK;
    }

    int64_t SocketUdp::sync_send(const unsigned char* data, size_t data_size) {
        if (!_pimpl->_opened) {
            return SOCKET_STATUS_FAILED;
        }

        if (!_pimpl->_connected) {
            return SOCKET_STATUS_FAILED;
        }

        auto res = SEND(_pimpl->_socket, reinterpret_cast<const char*>(data), SIZE_CAST(data_size), 0);

        return res >= 0 ? res : static_cast<int64_t>(SOCKET_STATUS_FAILED);
    }

    int SocketUdp::async_receive(string& data, size_t data_size, SocketCallback callback) {  // NOLINT(runtime/references)
        // Check whether the socket is opened or already receiving
        if (!_pimpl->_opened
            || _pimpl->_receiving) {
            return SOCKET_STATUS_FAILED;
        }

        // Lock
        unique_lock<mutex> socket_lock(_pimpl->_socket_mutex);

        // Initialize receiving task
        ThreadTask task = [this, &data, data_size, callback] () -> void { _pimpl->thread_receive_data(data, data_size, callback); };
        // Start thread with the task
        _pimpl->_sending_thread = unique_ptr<Thread>(new Thread(task));

        _pimpl->_receiving = true;

        return SOCKET_STATUS_OK;
    }

    int64_t SocketUdp::sync_receive(unsigned char* data, size_t data_size, unsigned int timeout) {
        if (!_pimpl->_opened)  {
            HUE_LOG << HUE_NETWORK << HUE_DEBUG << "SocketUdp: socket not open" << HUE_ENDL;
            return SOCKET_STATUS_FAILED;
        }

        if (!_pimpl->_connected) {
            HUE_LOG << HUE_NETWORK << HUE_DEBUG << "SocketUdp: socket not connected" << HUE_ENDL;
            return SOCKET_STATUS_FAILED;
        }

        if (data_size == 0) {
            HUE_LOG << HUE_NETWORK << HUE_DEBUG << "SocketUdp: buffer length is 0" << HUE_ENDL;
            return SOCKET_STATUS_FAILED;
        }

        fd_set set;
        // wait 100 milliseconds
        struct timeval tv_timeout;

        tv_timeout.tv_sec = timeout / 1000;
        tv_timeout.tv_usec = (timeout % 1000) * 1000;


        FD_ZERO(&set);
        FD_SET(_pimpl->_socket, &set);

        auto ret = SELECT(FD_SETSIZE, &set, nullptr, nullptr, timeout == 0 ? NULL : &tv_timeout);

        if (ret == 0) {
            HUE_LOG << HUE_NETWORK <<  HUE_DEBUG << "SocketUdp: socket timed out" << HUE_ENDL;
            return SOCKET_STATUS_TIMEOUT;
        }

        if (ret < 0) {
            if (errno == EINTR) {
                HUE_LOG << HUE_NETWORK <<  HUE_DEBUG << "SocketUdp: socket busy" << HUE_ENDL;
                return SOCKET_STATUS_AGAIN;
            }

            // select failed / socket invalid
            HUE_LOG << HUE_NETWORK << HUE_DEBUG << "SocketUdp: receiving data failed , msg: " << "socket failure, select() failed with code:" + to_string(errno) << HUE_ENDL;
            return SOCKET_STATUS_FAILED;
        }

        // socket ready for read

        // Take into account following QA for datagram socket:
        // Q: What happens if recv gets a packet larger than the buffer ?
        // A: The excess bytes are discarded.
        // Q: How can I know if I have received the entire message?
        // A: An single recv call always returns a single datagram.
        // and some more information:
        // For UDP you are always save if you use a buffer of 64 KB, since no IP packet (v4 or v6) can be above 64 KB in size (not even when fragmented) and
        // this even includes the headers IIRC, so data will always be below 64 KB for sure.
        // Receive data
        auto bytes_received = RECV(_pimpl->_socket, reinterpret_cast<char*>(data), SIZE_CAST(data_size), 0);
        if (bytes_received < 0) {
            HUE_LOG << HUE_NETWORK << HUE_DEBUG << "SocketUdp: receiving data failed , msg: " << "socket failure, recv() failed with code:" + to_string(errno) << HUE_ENDL;
            return SOCKET_STATUS_FAILED;
        }

        if (bytes_received > 0) {
            HUE_LOG << HUE_NETWORK <<  HUE_DEBUG << "SocketUdp: received data: " << data << HUE_ENDL;
        }

        return bytes_received;
    }

    int SocketUdp::async_receive(string& /*data*/, size_t /*data_size*/, const SocketAddress& /*address_remote*/, SocketCallback /*callback*/) {  // NOLINT(runtime/references)
        // Not supported yet
        return SOCKET_STATUS_FAILED;
    }

    int SocketUdp::get_reuse_address() const {
        int reuse_address = -1;

        if (_pimpl->_opened
            && !_pimpl->_bounded) {
            // Lock
            unique_lock<mutex> socket_lock(_pimpl->_socket_mutex);

            socklen_t size = sizeof(reuse_address);
            (void)GETSOCKOPT(_pimpl->_socket, SOL_SOCKET, SO_REUSEADDR, &reuse_address, &size);
        }

        return reuse_address;
    }

    int SocketUdp::set_reuse_address(int reuse_address) {
        int status = SOCKET_STATUS_FAILED;

        if (_pimpl->_opened
            && !_pimpl->_bounded) {
            // Lock
            unique_lock<mutex> socket_lock(_pimpl->_socket_mutex);

            status = SETSOCKOPT(_pimpl->_socket, SOL_SOCKET, SO_REUSEADDR, &reuse_address, sizeof(int));
        }

        return status;
    }

    int SocketUdp::get_broadcast() const {
        int broadcast = -1;

        if (_pimpl->_opened
            && !_pimpl->_bounded) {
            // Lock
            unique_lock<mutex> socket_lock(_pimpl->_socket_mutex);

            socklen_t size = sizeof(broadcast);
            (void)GETSOCKOPT(_pimpl->_socket, SOL_SOCKET, SO_BROADCAST, &broadcast, &size);
        }

        return broadcast;
    }

    int SocketUdp::set_broadcast(int broadcast) {
        int status = SOCKET_STATUS_FAILED;

        if (_pimpl->_opened
            && !_pimpl->_bounded) {
            // Lock
            unique_lock<mutex> socket_lock(_pimpl->_socket_mutex);

            status = SETSOCKOPT(_pimpl->_socket, SOL_SOCKET, SO_REUSEADDR, &broadcast, sizeof(int));
        }

        return status;
    }

    int SocketUdp::join_group(const SocketAddress& address_multicast) {
        int status = SOCKET_STATUS_FAILED;

        if (_pimpl->_opened
            && !_pimpl->_bounded) {
            // Lock
            unique_lock<mutex> socket_lock(_pimpl->_socket_mutex);

            ip_mreq address_multicast_req;
            if (INET_PTON(AF_INET, address_multicast.get_ip().c_str(), &address_multicast_req.imr_multiaddr.s_addr) != 1) {
                return SOCKET_STATUS_FAILED;
            }
            address_multicast_req.imr_interface.s_addr = htonl(INADDR_ANY);
            status = SETSOCKOPT(_pimpl->_socket, IPPROTO_IP, IP_ADD_MEMBERSHIP, &address_multicast_req, sizeof(address_multicast_req));
        }

        return status;
    }

    int SocketUdp::leave_group(const SocketAddress& address_multicast) {
        int status = SOCKET_STATUS_FAILED;

        if (_pimpl->_opened
            && !_pimpl->_bounded) {
            // Lock
            unique_lock<mutex> socket_lock(_pimpl->_socket_mutex);

            ip_mreq address_multicast_req;
            if (INET_PTON(AF_INET, address_multicast.get_ip().c_str(), &address_multicast_req.imr_multiaddr.s_addr) != 1) {
                return SOCKET_STATUS_FAILED;
            }
            address_multicast_req.imr_interface.s_addr = htonl(INADDR_ANY);
            status = SETSOCKOPT(_pimpl->_socket, IPPROTO_IP, IP_DROP_MEMBERSHIP, &address_multicast_req, sizeof(address_multicast_req));
        }

        return status;
    }

    void SocketUdp::Impl::thread_send_data(string data, support::SocketAddress address_remote, SocketCallback callback) {
        SocketError error("", SOCKET_STATUS_OK);

        const string ip         = address_remote.get_ip();
        const unsigned int port = address_remote.get_port();

        struct sockaddr_in socket_address_remote;
        socket_address_remote.sin_family = AF_INET;
        socket_address_remote.sin_port   = htons((u_short)port);

        if (INET_PTON(AF_INET, ip.c_str(), &socket_address_remote.sin_addr) != 1) {
            // TODO(Wies): check error handling
            error = SocketError("socket failure, INET_PTON() failed with code:" + to_string(errno), SOCKET_STATUS_FAILED);
            return;
        }

        int64_t bytes_left = static_cast<int64_t>(data.size());
        // Buffer to send
        const char* buffer = data.c_str();
        // Keep sending until all data in the buffer has been sent or socket closed
        while (bytes_left > 0) {
            // give chance other threads
            std::this_thread::yield();

            // Lock
            unique_lock<mutex> socket_lock_sending(_socket_mutex);

            // Check if socket is still open and no error occurred
            if (_opened
                && error.get_code() != SOCKET_STATUS_FAILED) {
                string buffer_str = string(buffer);

                // Send the data
                int64_t bytes_sent = SENDTO(_socket, buffer, SIZE_CAST(bytes_left), 0, (struct sockaddr*)&socket_address_remote, sizeof(socket_address_remote));

                if (bytes_sent > 0) {
                    // Calculate how many bytes still need to be sent
                    bytes_left -= bytes_sent;
                    // Move pointer of the buffer
                    buffer     += bytes_sent;

                    HUE_LOG << HUE_NETWORK <<  HUE_DEBUG << "SocketUdp: sent data chunk: " << buffer_str << ", bytes sent: " << bytes_sent << ", bytes left: " << bytes_left << HUE_ENDL;
                } else {
                    error = SocketError("socket failure, sendto() failed with code:" + to_string(errno), SOCKET_STATUS_FAILED);

                    HUE_LOG << HUE_NETWORK <<  HUE_DEBUG << "SocketUdp: sending data failed , msg: " << error.get_message() << HUE_ENDL;

                    break;
                }
            } else {
                error = SocketError("socket closed", SOCKET_STATUS_CLOSED);

                HUE_LOG << HUE_NETWORK <<  HUE_DEBUG << "SocketUdp: socket closed" << HUE_ENDL;

                break;
            }
        }

        /* callback */

        // Calculate the number of bytes sent
        size_t bytes_sent = data.size() - static_cast<size_t>(bytes_left);

        callback(static_cast<const SocketError&>(error), static_cast<unsigned int>(bytes_sent));

        // Done sending
        _sending = false;
    }

    void SocketUdp::Impl::thread_receive_data(string& data, size_t data_size, SocketCallback callback) {  // NOLINT
        SocketError error("", SOCKET_STATUS_OK);

        int64_t bytes_received = 0;

        if (data_size == 0) {
            error = SocketError("buffer length is 0", SOCKET_STATUS_FAILED);

            HUE_LOG << HUE_NETWORK <<  HUE_DEBUG << "SocketUdp: buffer length is 0" << HUE_ENDL;
        } else {
            fd_set set;
            // wait 100 milliseconds
            struct timeval timeout;

            timeout.tv_sec = 0;
            timeout.tv_usec = 100000;

            while (true) {
                if (!_opened) {
                    error = SocketError("socket closed", SOCKET_STATUS_CLOSED);

                    HUE_LOG << HUE_NETWORK <<  HUE_DEBUG << "SocketUdp: socket closed" << HUE_ENDL;
                    break;
                }

                unique_lock<mutex> socket_lock_receiving(_socket_mutex);

                FD_ZERO(&set);
                FD_SET(_socket, &set);

                int ret = SELECT(FD_SETSIZE, &set, nullptr, nullptr, &timeout);

                if (ret < 0) {
                    if (errno == EINTR) {
                        // interrupted by signal, retry
                        continue;
                    }

                    // select failed / socket invalid
                    error = SocketError("socket failure, select() failed with code:" + to_string(errno), SOCKET_STATUS_FAILED);

                    HUE_LOG << HUE_NETWORK << HUE_DEBUG << "SocketUdp: receiving data failed , msg: " << error.get_message() << HUE_ENDL;
                    break;
                }

                if (ret > 0) {
                    // socket ready for read

                    // Take into account following QA for datagram socket:
                    // Q: What happens if recv gets a packet larger than the buffer ?
                    // A: The excess bytes are discarded.
                    // Q: How can I know if I have received the entire message?
                    // A: An single recv call always returns a single datagram.
                    // and some more information:
                    // For UDP you are always save if you use a buffer of 64 KB, since no IP packet (v4 or v6) can be above 64 KB in size (not even when fragmented) and
                    // this even includes the headers IIRC, so data will always be below 64 KB for sure.
                    char buffer[SOCKET_BUFFER_SIZE];
                    // Receive data
                    bytes_received = RECV(_socket, reinterpret_cast<char*>(&buffer), SIZE_CAST(data_size), 0);

                    if (bytes_received > 0) {
                        // Copy the buffer to the data string
                        data = string(buffer, static_cast<size_t>(bytes_received));

                        HUE_LOG << HUE_NETWORK <<  HUE_DEBUG << "SocketUdp: received data: " << data << HUE_ENDL;
                    } else if (bytes_received < 0) {
                        error = SocketError("socket failure, recv() failed with code:" + to_string(errno), SOCKET_STATUS_FAILED);

                        HUE_LOG << HUE_NETWORK << HUE_DEBUG << "SocketUdp: receiving data failed , msg: " << error.get_message() << HUE_ENDL;
                    }

                    break;
                }
            }
        }

        // Done receiving
        _receiving = false;

        /* callback */
        unsigned int bytes = 0;
        if (bytes_received > 0) {
            bytes = static_cast<unsigned int>(bytes_received);
        }

        callback(static_cast<const SocketError&>(error), bytes);
    }

}  // namespace support
