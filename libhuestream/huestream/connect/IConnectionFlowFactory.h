/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#ifndef HUESTREAM_CONNECT_ICONNECTIONFLOWFACTORY_H_
#define HUESTREAM_CONNECT_ICONNECTIONFLOWFACTORY_H_

#include "huestream/connect/MessageDispatcher.h"
#include "huestream/connect/IBridgeSearcher.h"
#include "huestream/connect/IBridgeAuthenticator.h"
#include "huestream/connect/IBridgeStorageAccessor.h"
#include "bridgediscovery/BridgeInfo.h"
#include "huestream/connect/IFullConfigRetriever.h"

#include <memory>

namespace huestream {

        class IConnectionFlowFactory {
        public:
            virtual ~IConnectionFlowFactory() = default;

            virtual BridgeSearcherPtr CreateSearcher() = 0;

            virtual BridgeAuthenticatorPtr CreateAuthenticator() = 0;

            virtual MessageDispatcherPtr GetMessageDispatcher() = 0;

            virtual BridgeStorageAccessorPtr GetStorageAccesser() = 0;

            virtual ConfigRetrieverPtr CreateConfigRetriever(bool useForcedActivation = true, ConfigType configType = ConfigType::Full, bool useClipV2 = false) = 0;
        };

        typedef std::shared_ptr<IConnectionFlowFactory> ConnectionFlowFactoryPtr;

}  // namespace huestream

#endif  // HUESTREAM_CONNECT_ICONNECTIONFLOWFACTORY_H_
