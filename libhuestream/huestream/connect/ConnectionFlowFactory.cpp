/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <huestream/connect/ConnectionFlowFactory.h>
#include <huestream/connect/BridgeConfigRetriever.h>

#include <memory>

namespace huestream {

ConnectionFlowFactory::ConnectionFlowFactory(BridgeSettingsPtr bridgeSettings,
                                             BridgeHttpClientPtr http,
                                             MessageDispatcherPtr messageDispatcher,
                                             BridgeStorageAccessorPtr storageAccessor) :
    _bridgeSettings(bridgeSettings),
    _http(http),
    _messageDispatcher(messageDispatcher),
    _storageAccessor(storageAccessor) {
}

BridgeSearcherPtr ConnectionFlowFactory::CreateSearcher() {
    return std::make_shared<BridgeSearcher>(_bridgeSettings);
}

BridgeAuthenticatorPtr ConnectionFlowFactory::CreateAuthenticator() {
    return std::make_shared<Authenticator>(_http);
}

MessageDispatcherPtr ConnectionFlowFactory::GetMessageDispatcher() {
    return _messageDispatcher;
}

BridgeStorageAccessorPtr ConnectionFlowFactory::GetStorageAccesser() {
    return _storageAccessor;
}

ConfigRetrieverPtr ConnectionFlowFactory::CreateConfigRetriever(bool useForcedActivation, ConfigType configType, bool useClipV2) {
    if (configType == ConfigType::Full && useClipV2) {
        return std::make_shared<BridgeConfigRetriever>(_http, useForcedActivation);
    }

    return std::make_shared<ConfigRetriever>(_http, useForcedActivation, configType);
}

}  // namespace huestream
