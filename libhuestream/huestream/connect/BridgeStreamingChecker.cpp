/*******************************************************************************
Copyright (C) 2019 Signify Holding
All Rights Reserved.
********************************************************************************/

#include <huestream/connect/BridgeStreamingChecker.h>
#include <support/network/NetworkConfiguration.h>
#include <memory>
#include <cmath>

namespace huestream {

BridgeStreamingChecker::BridgeStreamingChecker(FullConfigRetrieverPtr fullConfigRetrieverPtr, const BridgeHttpClientPtr http)
    : _fullConfigRetrieverPtr(fullConfigRetrieverPtr),
          _http(http){
    _executor = support::make_unique<support::HttpRequestExecutor>(1);
    _executor->add_error_to_filter_on_retry(support::HttpRequestError::HTTP_REQUEST_ERROR_CODE_UNDEFINED);
}


void BridgeStreamingChecker::Check(BridgePtr bridge) {
    if (!bridge->IsConnectable()) {
        return;
    }

    std::weak_ptr<BridgeStreamingChecker> lifetime = shared_from_this();

        if (bridge->IsSupportingClipV2())
        {
            // With ClipV2 we only check for connectivity. Everything else will be handle by various server sent events.
            std::string url = bridge->GetAppIdUrl();
            std::weak_ptr<BridgeStreamingChecker> lifetime = shared_from_this();

            std::shared_ptr<support::HttpRequest> request = std::make_shared<support::HttpRequest>(url);
            request->set_content_type("application/json");
            request->add_header_field("hue-application-key", bridge->GetUser());
            request->set_verify_ssl(true);
            auto trusted_certificates = support::NetworkConfiguration::get_root_certificates();
            trusted_certificates.push_back(bridge->GetCertificate());
            request->set_trusted_certs(trusted_certificates);
            request->expect_common_name(support::to_lower_case(bridge->GetId()));

            _executor->add(request, support::HttpRequestExecutor::RequestType::REQUEST_TYPE_GET, [lifetime, bridge, this](const support::HttpRequestError& error, const support::IHttpResponse& response, std::shared_ptr<support::HttpRequestExecutor::IRequestInfo> request_info)
            {
                std::shared_ptr<BridgeStreamingChecker> ref = lifetime.lock();
                if (ref == nullptr)
                {
                    return;
                }

                static long lastLocalPort = 0;
                static std::string lastBridgeId = bridge->GetId();

                support::HttpRequestError::ErrorCode errorCode = error.get_code();

                // Local port is used to figured out if the single http2 connection we're using has been toss away by curl, which can happen when computer comes back from sleep.
                // Since the BridgeConfigRetriever eventing connection is never ending and we don't receive any notices from curl about that situation, we're left with a dead
                // connection that's never going to received any updates from the bridge anymore. By comparing this connection local port with the one from the previous request,
                // we can guess if that situation happened in which case both ports will be different. We then generate a fake disconnect event which will be picked up by the
                // BridgeConfigRetriever who will recreate a new eventing request and do a full refresh of the config from the bridge. Also make sure that this fake disconnect
                // event is not triggered when we've just connect to a new bridge.

                // The local port is an ephemeral port and those are temporary ports assigned by a machine's IP stack, and are assigned from a designated range of ports for this purpose.
                // When the connection terminates, the ephemeral port is available for reuse, although most IP stacks won't reuse that port number until the entire pool of ephemeral ports have been used.
                // So, if the client program reconnects, it will be assigned a different ephemeral port number for its side of the new connection.                
								long localPort = response.get_local_port();
                if (errorCode == support::HttpRequestError::HTTP_REQUEST_ERROR_CODE_SUCCESS && lastBridgeId == bridge->GetId() && lastLocalPort != 0 && lastLocalPort != localPort)
                {
                    errorCode = support::HttpRequestError::HTTP_REQUEST_ERROR_CODE_INVALID_EVENTING_CONNECTION;
                }

                bridge->SetLastHttpErrorCode(errorCode);
                bridge->SetLastHttpStatusCode(response.get_status_code());

                if (errorCode == support::HttpRequestError::HTTP_REQUEST_ERROR_CODE_SUCCESS && response.get_status_code() == 200)
                {
                    if (!bridge->IsConnected())
                    {
                        bridge->SetIsValidIp(true);
                        _messageCallback(FeedbackMessage(FeedbackMessage::REQUEST_TYPE_INTERNAL, FeedbackMessage::ID_BRIDGE_CONNECTED, bridge));
                    }

										// Don't assign an invalid port #, it seems to happen sometimes with curl. That doesn't mean the connection is invalid though.
										if (localPort > 0)
										{
											lastLocalPort = localPort;
                      lastBridgeId = bridge->GetId();
										}
                }
                else
                {
                    if (errorCode == support::HttpRequestError::HTTP_REQUEST_ERROR_CODE_INVALID_EVENTING_CONNECTION)
                    {
                        if (bridge->IsConnected())
                        {
                            // In this special case we only need to notify the config retriever about the issue and not whole app because the connection is still
                            // valid but the eventing connection in the config retriever is not anymore.
                            _messageCallback(FeedbackMessage(FeedbackMessage::REQUEST_TYPE_INTERNAL, FeedbackMessage::ID_INVALID_EVENTING_CONNECTION, bridge));
                            if (localPort > 0)
                            {
                                lastLocalPort = localPort;
                            }
                        }
                        else
                        {
                          lastLocalPort = 0;
                        }
                    }
                    else
                    {
                        if (bridge->IsConnected())
                        {
                            // Stop streaming first otherwise client app might stop streaming on the bridge disconnect event and since the bridge is going to have an invalid ip
                            // streaming won't be really stopped in HueStream::Stop(), which will cause issue later such as being unable to restart syncing.
                            if (bridge->IsStreaming())
                            {
                                 bridge->GetGroup()->SetActive(false);
                                 bridge->GetGroup()->SetOwner("");
                                _messageCallback(FeedbackMessage(FeedbackMessage::REQUEST_TYPE_INTERNAL, FeedbackMessage::ID_STREAMING_DISCONNECTED, bridge));
                            }

                            bridge->SetIsValidIp(false);
                            _messageCallback(FeedbackMessage(FeedbackMessage::REQUEST_TYPE_INTERNAL, FeedbackMessage::ID_BRIDGE_DISCONNECTED, bridge));
                        }

                        lastLocalPort = 0;
                    }
                }
            }, url.c_str());
        }
        else
        {
            _fullConfigRetrieverPtr->Execute(bridge->Clone(), [this, lifetime, bridge](OperationResult result, BridgePtr actualBridge) {
                  std::shared_ptr<BridgeStreamingChecker> ref = lifetime.lock();
                    if (ref == nullptr) {
                          return;
                    }

                    if (!bridge->IsConnected() && !actualBridge->IsConnected())
                          return;

                    auto differences = CompareBridges(bridge, actualBridge);
                    for (auto const& feedbackId : differences) {
                        _messageCallback(FeedbackMessage(FeedbackMessage::REQUEST_TYPE_INTERNAL, feedbackId, actualBridge));
                    }
            }, [](const huestream::FeedbackMessage& msg)
            {
            });
        }
}

void BridgeStreamingChecker::SetFeedbackMessageCallback(std::function<void(const huestream::FeedbackMessage &)> callback) {
    _messageCallback = callback;
}

static bool IsBridgeChanged(BridgePtr oldBridge, BridgePtr newBridge);
static bool AreGroupsUpdated(GroupListPtr expectedGroups, GroupListPtr actualGroups);
static bool AreLightsUpdated(LightListPtr expectedLights, LightListPtr actualLights);
static bool IsLightUpdated(LightPtr expectedLight, LightPtr actualLight);
static bool IsPositionLightUpdated(const Location &expectedLightPosition, const Location &actualLightPosition);
static bool IsLightColorUpdated(const Color &expectLightColor, const Color &actualLightColor);
static bool IsHomeAutomationStateUpdated(GroupPtr expectedGroup, GroupPtr actualGroup);

std::vector<FeedbackMessage::Id> CompareBridges(BridgePtr oldBridge, BridgePtr newBridge) {
    std::vector<FeedbackMessage::Id> differences;

    if (!oldBridge->IsConnected() && newBridge->IsConnected()) {
        differences.push_back(FeedbackMessage::ID_BRIDGE_CONNECTED);
    }
        if (oldBridge->IsConnected() && !newBridge->IsConnected())
        {
            differences.push_back(FeedbackMessage::ID_BRIDGE_DISCONNECTED);
        }

    if (!oldBridge->IsStreaming() && newBridge->IsStreaming()) {
        differences.push_back(FeedbackMessage::ID_STREAMING_CONNECTED);
    }
    if (oldBridge->IsStreaming() && !newBridge->IsStreaming()) {
        differences.push_back(FeedbackMessage::ID_STREAMING_DISCONNECTED);
    }

    if (IsBridgeChanged(oldBridge, newBridge)) {
        differences.push_back(FeedbackMessage::ID_BRIDGE_CHANGED);
    }

    if (AreGroupsUpdated(oldBridge->GetGroups(), newBridge->GetGroups())) {
        differences.push_back(FeedbackMessage::ID_GROUPLIST_UPDATED);
    }

    if (AreLightsUpdated(oldBridge->GetGroupLights(), newBridge->GetGroupLights())) {
        differences.push_back(FeedbackMessage::ID_LIGHTS_UPDATED);
    }

    if (!newBridge->IsStreaming() && IsHomeAutomationStateUpdated(oldBridge->GetGroup(), newBridge->GetGroup())) {
        differences.push_back(FeedbackMessage::ID_GROUP_LIGHTSTATE_UPDATED);
    }

    return differences;
}

static bool IsBridgeChanged(BridgePtr oldBridge, BridgePtr newBridge) {
    return (oldBridge->GetId() != newBridge->GetId() ||
        oldBridge->GetName() != newBridge->GetName() ||
        oldBridge->GetIpAddress() != newBridge->GetIpAddress() ||
        oldBridge->GetModelId() != newBridge->GetModelId() ||
        oldBridge->GetApiversion() != newBridge->GetApiversion() ||
        oldBridge->GetUser() != newBridge->GetUser() ||
        oldBridge->GetClientKey() != newBridge->GetClientKey());
}

static bool AreGroupsUpdated(GroupListPtr expectedGroups, GroupListPtr actualGroups) {
    if (actualGroups->size() != expectedGroups->size()) {
        return true;
    }

    for (size_t i = 0; i < actualGroups->size(); ++i) {
        if (expectedGroups->at(i)->GetId() != actualGroups->at(i)->GetId() ||
            expectedGroups->at(i)->GetName() != actualGroups->at(i)->GetName() ||
            expectedGroups->at(i)->Active() != actualGroups->at(i)->Active() ||
            expectedGroups->at(i)->GetOwner() != actualGroups->at(i)->GetOwner()) {
            return true;
        }
    }
    return false;
}

static bool AreLightsUpdated(LightListPtr expectedLights, LightListPtr actualLights) {
    if (actualLights->size() != expectedLights->size()) {
        return true;
    }

    for (size_t i = 0; i < actualLights->size(); ++i) {
        if (IsLightUpdated(expectedLights->at(i), actualLights->at(i))) {
            return true;
        }
    }
    return false;
}

static bool IsLightUpdated(LightPtr expectedLight, LightPtr actualLight) {
    return expectedLight->GetId() != actualLight->GetId() ||
        IsPositionLightUpdated(expectedLight->GetPosition(), actualLight->GetPosition()) ||
        IsLightColorUpdated(expectedLight->GetColor(), actualLight->GetColor()) ||
        expectedLight->Reachable() != actualLight->Reachable();
}

static bool IsPositionLightUpdated(const Location &expectedLightPosition,
    const Location &actualLightPosition) {
    return expectedLightPosition.GetX() != actualLightPosition.GetX() ||
        expectedLightPosition.GetY() != actualLightPosition.GetY();
}

static bool IsLightColorUpdated(const Color &expectLightColor, const Color &actualLightColor) {
    return (std::abs(expectLightColor.GetR() - actualLightColor.GetR()) > 0.0274f || std::abs(expectLightColor.GetG() - actualLightColor.GetG()) > 0.0274f || std::abs(expectLightColor.GetB() - actualLightColor.GetB()) > 0.0274f);
}

static bool IsHomeAutomationStateUpdated(GroupPtr expectedGroup, GroupPtr actualGroup) {
    if (actualGroup == nullptr) {
        return false;
    }
    if (expectedGroup == nullptr) {
        return true;
    }
    return expectedGroup->OnState() != actualGroup->OnState() ||
        expectedGroup->GetBrightnessState() != actualGroup->GetBrightnessState();
}

}  // namespace huestream
