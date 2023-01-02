/*******************************************************************************
Copyright (C) 2019 Signify Holding
All Rights Reserved.
********************************************************************************/
#pragma once

#include <future>
#include <memory>
#include <map>
#include <vector>

#include "support/network/http/IHttpResponse.h"
#include "support/network/http/HttpRequestError.h"
#include "support/util/File.h"
#include "support/threading/ThreadPoolExecutor.h"

namespace support {
    class HttpRequest;

    class HttpRequestExecutor{
    public:
        class IRequestInfo {
        public:
            virtual ~IRequestInfo() = default;
            virtual std::shared_ptr<HttpRequest> get_request() = 0;
            virtual int retry_countdown() = 0;
        };

        class RequestInfo;

        enum class HttpErrorPostActionType {
            NONE,
            RETRY,
            DISCARD
        };

        struct HttpErrorPostAction {
            HttpErrorPostActionType type;
            std::chrono::milliseconds delay = std::chrono::milliseconds{0};
            unsigned int min_retries = 0;
        };

        enum class RequestType {
            REQUEST_TYPE_GET,
            REQUEST_TYPE_PUT,
            REQUEST_TYPE_POST,
            REQUEST_TYPE_DELETE
        };

        /**
         Callback
         @param error the http request error that occurred (or HTTP_REQUEST_ERROR_CODE_SUCCESS)
         @param response the http response from the server
         @param request_info can be used to execute the same request again, by calling add(request_info)
         */
        using Callback = std::function<void(const support::HttpRequestError& error, const IHttpResponse& response, std::shared_ptr<support::HttpRequestExecutor::IRequestInfo> request_info)>;

        using HttpErrorDelegate = std::function<HttpErrorPostAction(support::HttpRequestError& error, const IHttpResponse& response, const std::shared_ptr<IRequestInfo>& request_info, const std::vector<HttpRequestError::ErrorCode>& errors_to_filter)>;

        /**
         constructor
         @param max_retries number of times to retry when a request fails
        */
        explicit HttpRequestExecutor(int max_retries);

        /**
        destructor
        */
        ~HttpRequestExecutor();

        /**
         constructor
         @param max_retries number of times to retry when a request fails
         */
        void set_max_retries(int max_retries);

        /**
         * Replace default http error delegate by custom implementation
         * @param retry_handler
         */
        void set_http_error_delegate(HttpErrorDelegate http_error_delegate);

        /**
         * cancel requests and stop executor
         */
        void stop();

        /**
         * cancel requests
         */
        void wait_for_pending_requests();

        /**
         add a request to be executed
         @param request the HttpRequest to preform
         @param request_type the type of request: get, put, post or delete
         @param callback the callback to be called after the request has finished
         @param resource_path the URL of the request
         @param body the HTTP body to send with the request (optional)
         @param file the file to send with the request (optional)
         @return whether the operation succeded (for ex. if the executor is not started)
         */
        bool add(std::shared_ptr<HttpRequest> request, RequestType request_type, Callback callback, const char* resource_path, const char* body = nullptr, File* file = nullptr);

        /**
         add a request to be executed by giving the request_info object that was recieved on callback of a previous execution of the request
         @param request_info request info object
         @return whether the operation succeded (for ex. if the executor is not started)
         */
        bool add(std::shared_ptr<IRequestInfo> request_info);

        /**
         disable auto removal after dispatching completion event.
         Disabling auto removal feature is useful when we want to extend lifespan of this object, as example to
         postpone dispatching the results to the end user. By calling this api the caller gets the responsibility for
         removing this object from executor.
         Note: destructor waits all requests to be done before destroying the object. If user forgets to remove object manually
               after calling this API then destructor will block forever.
         * @param request_info request info object
         */
        void disable_auto_removal(std::shared_ptr<IRequestInfo> request_info);

        /**
         remove request info object.
         * @param request_info request info object
         */
        void remove(std::shared_ptr<IRequestInfo> request_info);

        /**
         notify that a request has been canceled (only to be called by HttpRequest)
         @param request the request that was canceled
         */
        void request_canceled(HttpRequest* request);

        /**
         add error to filter in on retry.
         @param error, the error for which to retry
        */
        void add_error_to_filter_on_retry(HttpRequestError::ErrorCode error);

    private:
        using RequestFuture = std::future<RequestInfo>;

        static const char* REQUEST_METHOD[];

        ThreadPoolExecutor _thread_pool_executor;
        std::atomic<int>            _max_retries;
        std::atomic<bool>           _stopped;
        std::mutex                  _request_map_mutex;
        std::condition_variable     _request_map_cond;
        std::map<std::shared_ptr<HttpRequest>, std::shared_ptr<IRequestInfo>> _request_map;
        HttpErrorDelegate _http_error_delegate;
        std::vector<HttpRequestError::ErrorCode> errors_to_filter;

        static HttpErrorPostAction handle_http_error(support::HttpRequestError& error, const support::IHttpResponse& response, const std::shared_ptr<support::HttpRequestExecutor::IRequestInfo>& request_info, const std::vector<HttpRequestError::ErrorCode>& errors_to_filter);

        /**
         execute an HTTP request
         @request_info all required information to perform an HTTP request
         */
        void execute(std::shared_ptr<IRequestInfo> request_info);

        /**
         handle the response of an HTTP request
         @param error an error that occurred during the request or HTTP_REQUEST_ERROR_CODE_SUCCESS
         @param response the HTTP response of the server
         @param request_info all information to (re-)execute the request
         */
        void handle_response(const HttpRequestError& error, const IHttpResponse& response, std::shared_ptr<IRequestInfo> request_info);

        /**
         add a request to the map of requests
         */
        void add_request(std::shared_ptr<HttpRequest> request, std::shared_ptr<IRequestInfo> request_info);

        /**
         remove a request from the map of requests
         */
        void remove_request(std::shared_ptr<HttpRequest> request);
        /**
         get request from the map of requests
         */
         std::shared_ptr<HttpRequest> get_request(std::shared_ptr<IRequestInfo> request_info);
    };


}  // namespace support

