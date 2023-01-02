/*******************************************************************************
 Copyright (C) 2018 Philips Lighting Holding B.V.
 All Rights Reserved.
 ********************************************************************************/

#include <chrono>
#include <functional>
#include <string>
#include <thread>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

#include "support/mock/network/http/MockHttpRequest.h"
#include "support/threading/QueueDispatcher.h"

using std::bind;
using std::string;
using std::unique_lock;
using std::unique_ptr;
using support::HTTP_REQUEST_STATUS_FAILED;
using support::HTTP_REQUEST_STATUS_OK;
using support::ThreadTask;
using support::QueueDispatcher;

namespace support_unittests {

    MockHttpRequest::~MockHttpRequest() {
        stop_thread_request();
    }
    
    int MockHttpRequest::default_do_get(HttpRequestCallback callback) {
        string method = "GET";
        string body   = "";
        
        // Do a fake get request on the url
        return do_request(method, body, nullptr, callback);
    }
    
    int MockHttpRequest::default_do_post(const string& body, File* file, HttpRequestCallback callback) {
        string method = "POST";
        
        // Do a fake post request on the url
        return do_request(method, body, file, callback);
    }
    
    int MockHttpRequest::default_do_put(const string& body, HttpRequestCallback callback) {
        string method = "PUT";
    
        // Do a fake put request on the url
        return do_request(method, body, nullptr, callback);
    }

    int MockHttpRequest::default_do_delete(HttpRequestCallback callback) {
        string method = "DELETE";
        string body   = "";
    
        // Do a fake delete request on the url
        return do_request(method, body, nullptr, callback);
    }

    void MockHttpRequest::stop_thread_request() {
        unique_lock<mutex> executing_lock(_executing_mutex);

        if (_executing_thread != nullptr) {
            _executing_thread->join();
            _executing_thread = nullptr;
        }

        if (_dispatcher) {
            _dispatcher->shutdown();
        }
    }

    void MockHttpRequest::start_thread_request(const string& method, const string& body, File* /*file*/, HttpRequestCallback callback) {
        stop_thread_request();

        unique_lock<mutex> executing_lock(_executing_mutex);

        _dispatcher = unique_ptr<QueueDispatcher>(new QueueDispatcher());

        // Initialize request task
        ThreadTask task = bind(&MockHttpRequest::thread_request, this, method, body, callback);
        // Start thread with the task
        _executing_thread = unique_ptr<Thread>(new Thread(task));
    }

    int MockHttpRequest::default_do_request(const string& method, const string& body, File* file, HttpRequestCallback callback) {
        int status = HTTP_REQUEST_STATUS_OK;

        if (!_executing) {
            _executing = true;
            start_thread_request(method, body, file, callback);
        } else {
            status = HTTP_REQUEST_STATUS_FAILED;
        }
        
        return status;
    }
    
    void MockHttpRequest::default_cancel() {
        if (_cancel) {
            return;
        }

        _cancel = true;
        stop_thread_request();

        if (_executor != nullptr) {
            _executor->request_canceled(this);
        }
    }

    void MockHttpRequest::thread_request(string method, string body, HttpRequestCallback callback) {
        fake_process(get_request_params(method, body, nullptr));

        // Get fake delay
        unsigned int delay     = fake_delay();
        // Get fake error
        HttpRequestError error = fake_error();
        // Get fake response
        HttpResponse response  = fake_response();

        // Wait for a while before calling the callback
#ifdef _WIN32
        // try to fix random hang on sleep_for
        ::Sleep(delay);
#else
        std::this_thread::sleep_for(std::chrono::milliseconds(delay));
#endif
        
        _executing = false;
        
        /* callback */

        _dispatcher->post([callback, error, response] {
            callback((const class HttpRequestError&)error, (const HttpResponse)response);
        });
    }
    
    int MockHttpRequest::original_do_get(support::HttpRequestCallback callback) {
        return HttpRequest::do_get(callback);
    }
    
    /**
     
     */
    int MockHttpRequest::original_do_post(const string& body, File* file, HttpRequestCallback callback) {
        return HttpRequest::do_post(body, file, callback);
    }
    
    /**
     
     */
    int MockHttpRequest::original_do_put(const string& body, HttpRequestCallback callback) {
        return HttpRequest::do_put(body, callback);
    }
    
    /**
     
     */
    int MockHttpRequest::original_do_delete(HttpRequestCallback callback) {
        return HttpRequest::do_delete(callback);
    }
    
    void MockHttpRequest::default_set_executor(support::HttpRequestExecutor* executor) {
        _executor = executor;
    }

    void MockHttpRequest::default_set_certificate_chain(const std::vector<std::string>& chain) {
        HttpRequestBase::set_trusted_certs(chain);
    }

    void MockHttpRequest::default_set_expected_common_name(const std::string& name) {
        HttpRequestBase::expect_common_name(name);
    }

    void MockHttpRequest::default_set_verify_ssl(bool verify) {
        HttpRequestBase::set_verify_ssl(verify);
    }

}  // namespace support_unittests

