/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#pragma once

#include <gmock/gmock.h>

#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "support/threading/Thread.h"
#include "support/threading/QueueDispatcher.h"
#include "support/network/http/HttpRequest.h"
#include "support/network/http/HttpRequestError.h"
#include "support/network/http/HttpResponse.h"
#include "support/network/http/HttpRequestExecutor.h"

using support::File;
using support::HttpRequest;
using support::HttpRequestExecutor;
using support::HttpRequestCallback;
using support::HttpRequestError;
using support::HttpRequestParams;
using support::HttpResponse;
using support::Thread;
using testing::Invoke;
using testing::Return;
using testing::_;

namespace support_unittests {

    class MockHttpRequest : public HttpRequest {
    public:
        explicit MockHttpRequest(const string& url, int connect_timeout = 8, int receive_timeout = 8, int request_timeout = 8) :
        HttpRequest(url, connect_timeout, receive_timeout, request_timeout),
        _url(url),
        _executing(false),
        _cancel(false),
        _executor(nullptr) {
            ON_CALL(*this, do_get(_))
                .WillByDefault(Invoke(this, &MockHttpRequest::default_do_get));
            ON_CALL(*this, do_post(_, _, _))
                .WillByDefault(Invoke(this, &MockHttpRequest::default_do_post));
            ON_CALL(*this, do_put(_, _))
                .WillByDefault(Invoke(this, &MockHttpRequest::default_do_put));
            ON_CALL(*this, do_delete(_))
                .WillByDefault(Invoke(this, &MockHttpRequest::default_do_delete));
            ON_CALL(*this, do_request(_, _, _, _))
                .WillByDefault(Invoke(this, &MockHttpRequest::default_do_request));
            ON_CALL(*this, fake_process(_))
                    .WillByDefault(Return());
            ON_CALL(*this, fake_delay())
                    .WillByDefault(Return(0));
            ON_CALL(*this, fake_error())
                    .WillByDefault(Return(HttpRequestError()));
            ON_CALL(*this, cancel())
                .WillByDefault(Invoke(this, &MockHttpRequest::default_cancel));
            ON_CALL(*this, set_executor(_))
                .WillByDefault(Invoke(this, &MockHttpRequest::default_set_executor));

            ON_CALL(*this, set_trusted_certs(_))
                    .WillByDefault(Invoke(this, &MockHttpRequest::default_set_certificate_chain));
            ON_CALL(*this, set_verify_ssl(_))
                    .WillByDefault(Invoke(this, &MockHttpRequest::default_set_verify_ssl));
            ON_CALL(*this, expect_common_name(_))
                    .WillByDefault(Invoke(this, &MockHttpRequest::default_set_expected_common_name));
        }

        MOCK_METHOD1(do_get, int(HttpRequestCallback callback));

        MOCK_METHOD3(do_post, int(const string& body, File* file, HttpRequestCallback callback));

        MOCK_METHOD2(do_put, int(const string& body, HttpRequestCallback callback));

        MOCK_METHOD1(do_delete, int(HttpRequestCallback callback));

        MOCK_METHOD4(do_request, int(const string& method, const string& body, File* file, HttpRequestCallback callback));

        MOCK_METHOD0(cancel, void());

        MOCK_METHOD1(set_executor, void(support::HttpRequestExecutor* executor));

        MOCK_METHOD2(add_header_field, void(const string& name, const string& value));

        MOCK_METHOD1(set_trusted_certs, void(const std::vector<std::string>&));

        MOCK_METHOD1(expect_common_name, void(const std::string&));

        MOCK_METHOD1(set_verify_ssl, void(bool));

        MOCK_METHOD1(get_private_network_interface_name_for_url, string(std::string url));

        /* internals */

        MOCK_METHOD0(fake_delay, unsigned int());

        MOCK_METHOD0(fake_error, HttpRequestError());

        MOCK_METHOD0(fake_response, HttpResponse());

        MOCK_METHOD1(fake_process, void(HttpRequestParams));

        int default_do_get(HttpRequestCallback callback);

        int default_do_post(const string& body, File* file, HttpRequestCallback callback);

        int default_do_put(const string& body, HttpRequestCallback callback);

        int default_do_delete(HttpRequestCallback callback);

        void default_cancel();

        int default_do_request(const string& method, const string& body, File* file, HttpRequestCallback callback);

        void default_set_executor(support::HttpRequestExecutor* executor);

        void default_set_certificate_chain(const std::vector<std::string>& chain);

        void default_set_expected_common_name(const std::string& name);

        void default_set_verify_ssl(bool verify);

        /* originals */

        int original_do_get(support::HttpRequestCallback callback);

        int original_do_post(const string& body, File* file, HttpRequestCallback callback);

        int original_do_put(const string& body, HttpRequestCallback callback);

        int original_do_delete(HttpRequestCallback callback);

//        bool ssl_verification_enabled();
//        std::string expected_common_name();
//        std::vector<std::string> certificate_chain();

        ~MockHttpRequest();

        HttpRequestParams get_request_data() const;

    protected:
        /** */
        std::string             _url;
        
        /** ensure thread safety handling the request */
        std::mutex              _executing_mutex;
        /** */
        std::atomic<bool>       _executing;
        /** thread which executes the request */
        std::unique_ptr<Thread> _executing_thread;
        /** */
        std::atomic<bool>       _cancel;
        /** the executor of this request */
        HttpRequestExecutor*    _executor;

        std::unique_ptr<support::QueueDispatcher> _dispatcher;

        HttpRequestParams       _request_data;

        /**
        
         */
        void thread_request(string method, string body, HttpRequestCallback callback);
        void start_thread_request(const string& method, const string& body, File* file, HttpRequestCallback callback);
        void stop_thread_request();
    };

}  // namespace support_unittests

