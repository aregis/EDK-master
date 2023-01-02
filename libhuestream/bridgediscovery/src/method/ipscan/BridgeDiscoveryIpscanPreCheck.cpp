 /*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <algorithm>
#include <chrono>
#include <string>
#include <utility>
#include <vector>

#include "support/logging/Log.h"
#include "support/network/sockets/bsd/SocketBsd.h"

#include "bridgediscovery/BridgeDiscoveryConst.h"

#include "method/ipscan/BridgeDiscoveryIpscanPreCheck.h"


namespace huesdk {

    class BridgeDiscoveryIpscanPreCheckNoop : public BridgeDiscoveryIpscanPreCheck {
    public:
        std::vector<std::string> filter_reachable_ips(
                std::vector<std::string> ips, const std::atomic<bool>&,
                const std::function<void(const std::string&)>&) override {
            return ips;
        }
    };

    std::shared_ptr<BridgeDiscoveryIpscanPreCheck> BridgeDiscoveryIpscanPreCheck::get_instance() {
        if (!_instance) {
#ifdef _WIN32
            _instance = std::make_shared<BridgeDiscoveryIpscanPreCheckNoop>();
#else
            _instance = std::make_shared<BridgeDiscoveryIpscanPreCheck>();
#endif
        }
        return _instance;
    }

    void BridgeDiscoveryIpscanPreCheck::set_instance(std::shared_ptr<BridgeDiscoveryIpscanPreCheck> instance) {
        _instance = instance;
    }

    std::shared_ptr<BridgeDiscoveryIpscanPreCheck> BridgeDiscoveryIpscanPreCheck::_instance;

    enum SocketStatus {
        SOCKETSTATUS_DISCONNECTED,
        SOCKETSTATUS_CONNECTING,
        SOCKETSTATUS_CONNECTED,
        SOCKETSTATUS_CONNECTION_ERROR,
        SOCKETSTATUS_SOCKET_ERROR,
        SOCKETSTATUS_UNEXPECTED_ERROR,
    };

    // a small struct to hold information about connection attempts
    struct AsyncSocket {
        SOCKET_ID _fd;
        std::string _ip;
        SocketStatus _status;
        std::chrono::steady_clock::time_point _timeout;
    };

    using socket_unique_ptr = std::unique_ptr<AsyncSocket, std::function<void(AsyncSocket*)>>;

    static SocketStatus async_connect(SOCKET_ID fd, const std::string ip) {
        if (SETNONBLOCKING(fd) == -1) {
            return SOCKETSTATUS_UNEXPECTED_ERROR;
        }

        struct sockaddr_in serv_addr;
        memset(&serv_addr, 0, sizeof serv_addr);
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(bridge_discovery_const::IPCHECK_HTTP_PORT);

        if (INET_PTON(AF_INET, ip.c_str(), &serv_addr.sin_addr) != 1) {
            return SOCKETSTATUS_UNEXPECTED_ERROR;
        }

        // start connecting
        if (CONNECT(fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) == 0) {
            // this should never happen because the socket is asynchronous
            return SOCKETSTATUS_CONNECTED;
        }

        if (ERRNO != SOCKET_INPROGRESS) {
            return SOCKETSTATUS_UNEXPECTED_ERROR;
        }

        return SOCKETSTATUS_CONNECTING;
    }

    /**
     Check a list of sockets for activity with select() for up to 100 milliseconds
     Upon completion, the struct's _status member will be updated
    */
    static void check_sockets(std::vector<socket_unique_ptr>* sockets) {
        fd_set rset;
        fd_set wset;
        fd_set eset;

        FD_ZERO(&rset);
        FD_ZERO(&wset);
        FD_ZERO(&eset);

        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 100000;  // 100 ms

        int max_fd = -1;

        for (auto& s : *sockets) {
            FD_SET(s->_fd, &rset);
            FD_SET(s->_fd, &wset);
            FD_SET(s->_fd, &eset);

#ifndef _WIN32
            if (s->_fd > max_fd) {
                max_fd = static_cast<int>(s->_fd);
            }
#endif
        }

        if (SELECT(max_fd + 1, &rset, &wset, &eset, &tv) == -1) {
            return;
        }

        for (auto& s : *sockets) {
            if (FD_ISSET(s->_fd, &eset) || FD_ISSET(s->_fd, &rset)) {
                s->_status = SOCKETSTATUS_CONNECTION_ERROR;
            } else if (FD_ISSET(s->_fd, &wset)) {
                s->_status = SOCKETSTATUS_CONNECTED;
            } else if (s->_timeout < std::chrono::steady_clock::now()) {
                s->_status = SOCKETSTATUS_CONNECTION_ERROR;
            }
        }
    }

    std::vector<std::string> BridgeDiscoveryIpscanPreCheck::filter_reachable_ips(
            std::vector<std::string> ips, const std::atomic<bool> &_stopped_by_user,
            const std::function<void(const std::string&)>& reachable_ip_found) {
        std::vector<socket_unique_ptr> active_sockets;
        std::vector<std::string> return_value;

        while (!_stopped_by_user.load() && (!ips.empty() || !active_sockets.empty())) {
            // add as many sockets as possible to the working set

            while (!ips.empty() && active_sockets.size() < bridge_discovery_const::IPSCAN_MAX_ACTIVE_SOCKETS) {
                SOCKET_ID fd = _SOCKET(AF_INET, SOCK_STREAM, 0);

                if (fd == INVALID_SOCKET) {
                    // could not create socket, try again later
                    break;
                }

                auto ip = ips.back();
                ips.pop_back();

                auto timeout = std::chrono::steady_clock::now() + std::chrono::seconds(bridge_discovery_const::IPCHECK_HTTP_CONNECT_TIMEOUT);

                auto status = async_connect(fd, ip);

                auto socket = socket_unique_ptr(new AsyncSocket{fd, ip, status, timeout}, [](AsyncSocket* s_) {
                    CLOSE(s_->_fd);
                    delete s_;
                });

                if (status == SOCKETSTATUS_UNEXPECTED_ERROR) {
                    // something went wrong when connecting, skip this ip
                    HUE_LOG << HUE_CORE << HUE_ERROR << "BridgeDiscoveryIpscan: socket error, skipping " << ip << HUE_ENDL;
                    continue;
                }

                active_sockets.push_back(std::move(socket));
            }

            check_sockets(&active_sockets);

            for (auto& s : active_sockets) {
                if (s->_status == SOCKETSTATUS_CONNECTED) {
                    return_value.push_back(s->_ip);
                    reachable_ip_found(s->_ip);
                }
            }

            active_sockets.erase(std::remove_if(active_sockets.begin(), active_sockets.end(), [](const socket_unique_ptr &s){
                return s->_status != SOCKETSTATUS_CONNECTING;
            }), active_sockets.end());
        }
        return return_value;
    }

}  // namespace huesdk
