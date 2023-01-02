/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <curl/curl.h>

#include <string>
#include <mutex>
#include <thread>

#include "support/network/http/curl/CurlHttpClient.h"
#include "support/network/http/curl/CurlRequest.h"
#include "support/network/http/HttpRequestParams.h"
#include "support/network/Network.h"
#include "support/network/NetworkConfiguration.h"

#define SHORT_WAIT_TIMEOUT_MS 100
#define LONG_WAIT_TIMEOUT_MS 10000

namespace support {
    /* curl doesnt know about threads, so we have to provide our own locking mechanism */

    static auto curl_mutex = std::make_shared<std::mutex>();

    static void share_lock(CURL*, curl_lock_data, curl_lock_access, void*) {
        curl_mutex->lock();
    }

    static void share_unlock(CURL*, curl_lock_data, curl_lock_access, void*) {
        curl_mutex->unlock();
    }

    static void share_cleanup(CURLSH* handle) {
        if (handle) {
            curl_share_cleanup(handle);
        }
    }

    /* share handle that shares the tls session information for all http requests */
    static std::unique_ptr<CURLSH, decltype(&share_cleanup)> _curlsh{nullptr, share_cleanup};

    CurlHttpClient::Message::Message(MessageType _type, CurlRequest* _request) :
            type(_type), request(_request) { }

    CurlHttpClient::CurlHttpClient() : _mutex{curl_mutex} {
        curl_global_init(CURL_GLOBAL_ALL);

        {
            std::lock_guard<std::mutex> lock(*_mutex);
            if (!_curlsh) {
                _curlsh.reset(curl_share_init());

                curl_share_setopt(_curlsh.get(), CURLSHOPT_LOCKFUNC, share_lock);
                curl_share_setopt(_curlsh.get(), CURLSHOPT_UNLOCKFUNC, share_unlock);
                curl_share_setopt(_curlsh.get(), CURLSHOPT_SHARE, CURL_LOCK_DATA_SSL_SESSION);
            }
        }

        _curlm = curl_multi_init();
        curl_multi_setopt(_curlm, CURLMOPT_PIPELINING, NetworkConfiguration::use_http2() ? CURLPIPE_MULTIPLEX : CURLPIPE_HTTP1);

        _thread = std::thread(&CurlHttpClient::thread_method, this);

        _signal_fds[0] = INVALID_SOCKET;
        _signal_fds[1] = INVALID_SOCKET;

#ifndef _WIN32
        socketpair(PF_LOCAL, SOCK_STREAM, 0, _signal_fds);
#endif
    }

    CurlHttpClient::~CurlHttpClient() {
        _queue.push_back(Message(Message::STOP_EXECUTOR));
        wake_up_executor();
        _thread.join();

        curl_multi_cleanup(_curlm);

        for (auto fd : _signal_fds) {
            if (fd != INVALID_SOCKET) {
                closesocket(fd);
            }
        }
    }

#ifdef ANDROID
    static curl_socket_t open_curl_socket(void *custom_data, curlsocktype purpose, struct curl_sockaddr *address) {
        (void) purpose;
        (void) address;
        auto curl_request = static_cast<CurlRequest *>(custom_data);
        auto socket_fd = socket(AF_INET, SOCK_STREAM, 0);
        Network::set_interface_for_socket(static_cast<int>(socket_fd), curl_request->get_interface_name());

        return socket_fd;
    }

    static int close_curl_socket(void *custom_data, curl_socket_t item) {
        (void)custom_data;
        int result = 0;
        if (item != CURL_SOCKET_BAD) {
            if (close(item) != 0) {
                HUE_LOG << HUE_NETWORK << HUE_ERROR << "CurlRequest: Closing socket " << item << " failed with error " << errno
                        << HUE_ENDL;
                result = 1;
            }
        }
        return result;
    }

    IHttpClient::Handle CurlHttpClient::start_request(const HttpRequestParams &data, HttpRequestCallback callback) {
        CurlRequest *request = new CurlRequest(data, callback);

        // run on specified interface
        if (!data.interface_name.empty()) {
            curl_easy_setopt(request->get_handle(), CURLOPT_OPENSOCKETFUNCTION, open_curl_socket);
            curl_easy_setopt(request->get_handle(), CURLOPT_OPENSOCKETDATA, request);
            curl_easy_setopt(request->get_handle(), CURLOPT_CLOSESOCKETFUNCTION, close_curl_socket);
        }

        return start_request(request);
    }
#else
    IHttpClient::Handle CurlHttpClient::start_request(const HttpRequestParams &data, HttpRequestCallback callback) {
        if (!data.interface_name.empty()) {
            assert("Specifying interface name is not supported on this platform");
        }
        return start_request(new CurlRequest(data, callback));
    }
#endif

    IHttpClient::Handle CurlHttpClient::start_request(CurlRequest *request) {
        // make this request use the shared tls data
        curl_easy_setopt(request->get_handle(), CURLOPT_SHARE, _curlsh.get());

        // back reference from the curl handle to the request object
        curl_easy_setopt(request->get_handle(), CURLOPT_PRIVATE, request);

        _queue.push_back(Message(Message::START_REQUEST, request));
        wake_up_executor();

        return request;
    }

    void CurlHttpClient::stop_request(IHttpClient::Handle handle) {
        CurlRequest* request = static_cast<CurlRequest*>(handle);
        _queue.push_back(Message(Message::STOP_REQUEST, request));
        wake_up_executor();
				request->wait_for_completion();
        delete request;
    }

    void CurlHttpClient::wake_up_executor() {
        if (_signal_fds[0] != INVALID_SOCKET) {
            char tmp_buf[1] = {};
            int err = send(_signal_fds[0], tmp_buf, 1, 0);
            if (err == -1) {
              HUE_LOG << HUE_NETWORK << HUE_ERROR << "CurlHttpClient: send returned -1 errno is " << errno << HUE_ENDL;
            }
        }
    }

    void CurlHttpClient::thread_method() {
        int num_active = 0;

        while (true) {
            if (num_active == 0) {
                // block until a new request arrives
                _queue.wait_for_data();
            }

            while (true) {
                Message message = _queue.pop_front();

                if (message.type == Message::NONE) {
                    break;
                }

                if (message.type == Message::START_REQUEST) {
                    curl_multi_add_handle(_curlm, message.request->get_handle());
                }

                if (message.type == Message::STOP_REQUEST) {
                    // we can tell if this request is still active by checking if the curl handle still exists
                    CURL *handle = message.request->get_handle();
                    if (handle != nullptr) {
                        curl_multi_remove_handle(_curlm, handle);
                        message.request->send_response(CURLE_ABORTED_BY_CALLBACK);
                    }
                    message.request->set_as_complete();
                }

                if (message.type == Message::STOP_EXECUTOR) {
                    return;
                }
            }

            if (_signal_fds[1] == INVALID_SOCKET) {
                curl_multi_wait(_curlm, nullptr, 0, SHORT_WAIT_TIMEOUT_MS, nullptr);
            } else {
                struct curl_waitfd extra_fds[1];

                extra_fds[0].fd = _signal_fds[1];
                extra_fds[0].events = CURL_WAIT_POLLIN;
                extra_fds[0].revents = 0;

                curl_multi_wait(_curlm, extra_fds, 1, LONG_WAIT_TIMEOUT_MS, nullptr);
                if (extra_fds[0].revents) {
                    // receive any data sent to the signaling socket
                    char tmp_buf[8] = {};
                    int err = recv(_signal_fds[1], tmp_buf, 8, 0);
                    if (err == -1) {
                      HUE_LOG << HUE_NETWORK << HUE_ERROR << "CurlHttpClient: recv returned -1 errno is " << errno << HUE_ENDL;
                    }
                }
            }

            curl_multi_perform(_curlm, &num_active);

            int msgs_in_queue = 0;
            do {
                struct CURLMsg *msg = curl_multi_info_read(_curlm, &msgs_in_queue);

                if (msg != nullptr && msg->msg == CURLMSG_DONE) {
                    curl_multi_remove_handle(_curlm, msg->easy_handle);

                    CurlRequest *request = nullptr;
                    curl_easy_getinfo(msg->easy_handle, CURLINFO_PRIVATE, &request);
                    request->send_response(msg->data.result);
                }
            } while (msgs_in_queue > 0);
        }
    }
}  // namespace support
