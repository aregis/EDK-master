/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include "huestream/common/http/BridgeHttpClient.h"

#include <memory>
#include <utility>

#include "huestream/common/data/Bridge.h"
#include "huestream/common/data/ApiVersion.h"

#include "support/threading/QueueExecutor.h"
#include "support/util/DeleteLater.h"
#include "support/network/NetworkConfiguration.h"
#include "support/network/http/HttpRequestConst.h"
#include "support/logging/Log.h"

namespace huestream {
    BridgeHttpClient::BridgeHttpClient(HttpClientPtr httpClient) : _httpClient(std::move(httpClient)) {
    }

    HttpRequestPtr BridgeHttpClient::ExecuteHttpRequest(BridgePtr bridge, const std::string& method, const std::string& url, const std::string& body, bool supportEvent) {
        if (bridge == nullptr) {
            return nullptr;
        }

        auto request = CreateRequest(bridge, method, url, body, supportEvent);
        ExecuteRequestInternal(std::move(bridge), request, {});

        request->WaitUntilReady();
        return request;
    }

    int32_t BridgeHttpClient::ExecuteHttpRequest(BridgePtr bridge, const std::string& method, const std::string& url, const std::string& body, HttpRequestCallback callback, bool supportEvent) {
        if (bridge == nullptr) {
            return -1;
        }

        auto request = CreateRequest(bridge, method, url, body, supportEvent);
        return ExecuteRequestInternal(std::move(bridge), std::move(request), std::move(callback));
    }

        void BridgeHttpClient::CancelHttpRequest(int32_t requestId)
        {
            _httpClient->CancelAsyncRequest(requestId);
        }

    HttpRequestPtr BridgeHttpClient::CreateRequest(BridgePtr bridge, const std::string& method, const std::string& url, const std::string& body, bool supportEvent) const {
        auto request = std::make_shared<HttpRequestInfo>(method, url, body);

        if (bridge->GetCertificate().empty()) {
            request->SetSslVerificationEnabled(false);
        }
        else {
            auto trusted_certificates = support::NetworkConfiguration::get_root_certificates();
            trusted_certificates.push_back(bridge->GetCertificate());
            request->SetTrustedCertificates(trusted_certificates);
            request->SetExpectedCommonName(support::to_lower_case(bridge->GetId()));
            request->SetSslVerificationEnabled(true);
        }

        std::unordered_map<std::string, std::string> header;
        header[support::HTTP_HEADER_CONTENT_TYPE] = support::HTTP_CONTENT_TYPE_APPLICATION_JSON;

        if (bridge->IsSupportingClipV2()) {
            header["hue-application-key"] = bridge->GetUser();

            // Set request header field "Accept: text/event-stream" for ClipV2 request if requested. Also need to set timeout to 0 since it's a never ending connection.
            if (supportEvent) {
                header["Accept"] = "text/event-stream";
                request->SetRequestTimeout(0);
            }
        }

        request->SetHeader(header);

        return request;
    }

    HttpRequestCallback BridgeHttpClient::LinkHttpClientCallbackWithBridge(BridgePtr bridge, HttpRequestCallback original_callback) {
        return [bridge, original_callback](const support::HttpRequestError& error, const support::IHttpResponse& response) {
            auto&& chain = response.get_certificate_chain();
            if (bridge->GetCertificate().empty() && !chain.empty()) {
                bridge->SetCertificate(chain.back());
            }

            if (original_callback) {
                original_callback(error, response);
            }
        };
    }

    int32_t BridgeHttpClient::ExecuteRequestInternal(BridgePtr bridge, HttpRequestPtr request, HttpRequestCallback callback) {
        if (!request->SslVerificationEnabled()) {
            HUE_LOG << HUE_CORE << HUE_DEBUG << "BridgeHttpClient: Pinning: Connecting to a bridge without a pinned certificate: SSL verification is disabled." << HUE_ENDL;
        } else {
            HUE_LOG << HUE_CORE << HUE_DEBUG << "BridgeHttpClient: Pinning: Using common name for pinning: " << request->GetExpectedCommonName() << HUE_ENDL;
            HUE_LOG << HUE_CORE << HUE_DEBUG << "BridgeHttpClient: Pinning: Using persisted certificate for bridge " << bridge->GetId() << ", certificate: " << bridge->GetCertificate() << HUE_ENDL;
        }
        return _httpClient->ExecuteAsync(std::move(request), LinkHttpClientCallbackWithBridge(std::move(bridge), std::move(callback)));
    }
}  // namespace huestream
