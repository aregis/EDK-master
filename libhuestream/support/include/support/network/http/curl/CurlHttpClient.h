/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <curl/curl.h>

#include <thread>
#include <memory>
#include <string>

#include "support/network/http/curl/CurlRequest.h"
#include "support/network/http/HttpRequestParams.h"
#include "support/network/http/IHttpClient.h"
#include "support/network/sockets/bsd/SocketBsdCompat.h"
#include "support/threading/BlockingQueue.h"
#include "support/threading/ThreadPool.h"

namespace support {

    class CurlHttpClient : public IHttpClient {
    public:
        CurlHttpClient();

        ~CurlHttpClient();

        Handle start_request(const HttpRequestParams &data, HttpRequestCallback callback) override;

        void stop_request(Handle handle) override;

    protected:
        IHttpClient::Handle start_request(CurlRequest *request);

    private:
        /* multi handle that shares the connection pool for all http requests */
        CURLM* _curlm;

        /* long living thread that performs all http requests */
        std::thread _thread;

        /* mutex provided to curl library for locking critical sections */
        std::shared_ptr<std::mutex> _mutex;

        struct Message {
            enum MessageType {
                NONE,
                START_REQUEST,
                STOP_REQUEST,
                STOP_EXECUTOR
            };

            MessageType type;
            CurlRequest *request;

            explicit Message(MessageType _type = NONE, CurlRequest* _request = nullptr);
        };

        /* communication pipe between requesters and the background thread */
        BlockingQueue<Message> _queue;

        /* fds used to force curl_multi_wait to return immediately */
        SOCKET_ID _signal_fds[2];

        /* wake up the executor by sending data through the socket pair */
        void wake_up_executor();

        /* thread method that performs all http requests */
        void thread_method();
    };

}  // namespace support

