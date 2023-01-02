/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#ifndef HUESTREAM_CONNECT_CONNECTIONFLOWFACTORY_H_
#define HUESTREAM_CONNECT_CONNECTIONFLOWFACTORY_H_

#include "huestream/connect/IConnectionFlowFactory.h"
#include "huestream/common/http/IBridgeHttpClient.h"
#include "huestream/connect/FullConfigRetriever.h"
#include "huestream/connect/BridgeSearcher.h"
#include "huestream/connect/Authenticator.h"

namespace huestream {


class ConnectionFlowFactory : public IConnectionFlowFactory {
 public:
    ConnectionFlowFactory(BridgeSettingsPtr bridgeSettings,
                          BridgeHttpClientPtr http,
                          MessageDispatcherPtr messageDispatcher,
                          BridgeStorageAccessorPtr storageAccessor);

    BridgeSearcherPtr CreateSearcher() override;

    BridgeAuthenticatorPtr CreateAuthenticator() override;

    MessageDispatcherPtr GetMessageDispatcher() override;

    BridgeStorageAccessorPtr GetStorageAccesser() override;

    ConfigRetrieverPtr CreateConfigRetriever(bool useForcedActivation = true, ConfigType configType = ConfigType::Full, bool useClipV2 = false) override;

 private:
    BridgeSettingsPtr _bridgeSettings;
    BridgeHttpClientPtr _http;
    MessageDispatcherPtr _messageDispatcher;
    BridgeStorageAccessorPtr _storageAccessor;
};

}  // namespace huestream

#endif  // HUESTREAM_CONNECT_CONNECTIONFLOWFACTORY_H_
