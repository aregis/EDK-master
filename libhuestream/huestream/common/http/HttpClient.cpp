/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <memory>

#include "huestream/common/http/HttpClient.h"

#include "support/network/http/_test/HttpRequestDelegator.h"
#include "support/threading/ThreadPool.h"
#include "support/util/DeleteLater.h"

#include <utility>

#define HTTP_REQUEST HttpRequestDelegator

namespace huestream {

struct IHttpClient::HttpRequest : public support::HTTP_REQUEST {
    using support::HTTP_REQUEST::HTTP_REQUEST;
};

void HttpClient::Execute(HttpRequestPtr request) {
    ExecuteAsync(request, {});
    request->WaitUntilReady();
}

HttpClient::~HttpClient() {
    while (true) {
        std::vector<std::tuple<int32_t, HttpRequest*>> active_requests;
        {
            std::lock_guard<std::mutex> lock{_data->_active_requests_mutex};
            _data->_is_shutdown = true;
            active_requests.swap(_data->_active_requests);
        }

        if (active_requests.empty()) {
            break;
        } else for (auto &&request : active_requests) {
            auto req = std::get<1>(request);
            req->cancel();
            delete req;
        }
    }
}

int32_t HttpClient::ExecuteAsync(HttpRequestPtr request, HttpRequestCallback callback) {
    auto thread_pool = support::GlobalThreadPool::get();
    if (!thread_pool) {
        return -1;
    }

    std::lock_guard<std::mutex> lock{_data->_active_requests_mutex};
    if (_data->_is_shutdown) {
        return -1;
    }

    if (!request->StartRequest()) {
        return -1;
    }

    HttpRequest* req = new HttpRequest(request->GetUrl());

    if (!request->GetToken().empty()) {
        req->set_bearer_auth_header(request->GetToken());
    }

    req->set_verify_ssl(request->SslVerificationEnabled());
    req->expect_common_name(request->GetExpectedCommonName());
    req->set_trusted_certs(request->GetTrustedCertificates());

    auto headerMap = request->GetHeader();
    bool isEventingRequest = false;
    for (auto headerIt = headerMap.begin(); headerIt != headerMap.end(); ++headerIt) {
      req->add_header_field(headerIt->first, headerIt->second);

      if (!isEventingRequest && headerIt->first == "Accept" && headerIt->second == "text/event-stream")
      {
        isEventingRequest = true;
      }
    }

    req->set_request_timeout(request->GetRequestTimeout());

    static int32_t globalRequestId = 0;
    int32_t requestId = globalRequestId;

    std::weak_ptr<Data> dataLifetime = _data;
    std::weak_ptr<HttpRequestInfo> requestLifetime = request;
    req->do_request(request->GetMethod(), request->GetBody(), nullptr,
        [request, requestLifetime, req, callback, thread_pool, dataLifetime, requestId, isEventingRequest](const support::HttpRequestError &error, const support::IHttpResponse &response) mutable {

            // Watch out here because request is captured by the lambda and its ref count is incremented by one. However its the only reference to it so
            // ref count is 1 at this point. So if the HttpClient object is deleted while the lambda is executing then that ref count is gonna reach 0
            // because the lambda is being destroy too. A lambda can be destroyed but execution of its code will continue and that might cause a crash
            // because the request is also destroy in the process. That's why we use an additional weak_ptr to make sure request still exist when
            // we're going to use it.
            std::shared_ptr<Data> dataRef = dataLifetime.lock();
            if (dataRef == nullptr)
            {
                return;
            }

            HttpRequestPtr requestRef = requestLifetime.lock();
            if (requestRef == nullptr)
            {
              return;
            }

            // At this point request should have a ref count of 2, thus making sure it won't be destroy until we're finished.
            requestRef->SetSuccess(false);

            if (error.get_code() == support::HttpRequestError::HTTP_REQUEST_ERROR_CODE_SUCCESS) {
                requestRef->SetResponse(response.get_body());
                requestRef->SetStatusCode(response.get_status_code());
                requestRef->SetSuccess(true);
            }

            if (dataRef->_is_shutdown) {
              return;
            }

            if (callback) {
                callback(error, response);
            }

            if (error.get_code() == support::HttpRequestError::HTTP_REQUEST_ERROR_CODE_CANCELED)
            {
              // Don't delete the request here, it's going to be taken care of in CancelAsyncRequest. Otherwise we'll end up in deadlock.
              requestRef->FinishRequest();
              return;
            }

            // Don't end server sent events request unless they failed. They are considered everlasting.
            if (!isEventingRequest || !requestRef->GetSuccess()) {
              requestRef->FinishRequest();
              {
                  std::lock_guard<std::mutex> lock{dataRef->_active_requests_mutex};
                  auto iter = std::find(dataRef->_active_requests.begin(), dataRef->_active_requests.end(), std::tuple<int32_t, HttpRequest*>(requestId, req));
                  if (iter != dataRef->_active_requests.end()) {
                      dataRef->_active_requests.erase(iter);
                      support::delete_later(req);
                  }
              }
            }
        });

    _data->_active_requests.emplace_back(std::tuple<int32_t, HttpRequest*>(globalRequestId++ , std::move(req)));

    return requestId;
}

void HttpClient::CancelAsyncRequest(int32_t requestId)
{
  HttpRequest* req = nullptr;
  {
    std::lock_guard<std::mutex> lock{ _data->_active_requests_mutex };
    for (auto requestIt = _data->_active_requests.begin(); requestIt != _data->_active_requests.end(); ++requestIt)
    {
      if (std::get<0>(*requestIt) == requestId)
      {
        req = std::get<1>(*requestIt);
        _data->_active_requests.erase(requestIt);
        break;
      }
    }
  }

  if (req != nullptr)
  {
    req->cancel();
    delete req;
  }
}

shared_ptr<IHttpClient::HttpRequest> HttpClient::CreateHttpRequest(const std::string& url,
                                                               int connect_timeout,
                                                               int receive_timeout,
                                                               int request_timeout,
                                                               bool enable_logging,
                                                               support::HttpRequestSecurityLevel security_level) {
    return std::make_shared<HttpRequest>(url, connect_timeout, receive_timeout, request_timeout, enable_logging, security_level);
}

}  // namespace huestream
