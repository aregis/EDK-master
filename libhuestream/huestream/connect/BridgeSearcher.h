/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#ifndef HUESTREAM_CONNECT_BRIDGESEARCHER_H_
#define HUESTREAM_CONNECT_BRIDGESEARCHER_H_

#include "huestream/connect/ConnectionFlow.h"
#include "bridgediscovery/BridgeDiscovery.h"

#include <vector>
#include <memory>

namespace huestream {

        class BridgeSearcher : public IBridgeSearcher, public huesdk::IBridgeDiscoveryCallback {
        public:
            explicit BridgeSearcher(BridgeSettingsPtr bridgeSettings);

            void SearchNew(SearchCallbackHandler cb) override;

            void SearchNew(bool bruteForce, SearchCallbackHandler cb) override;

            void Abort() override;

        protected:
            virtual void operator()(const std::vector<std::shared_ptr<huesdk::BridgeDiscoveryResult>> &results,
                                    huesdk::BridgeDiscoveryReturnCode return_code) override;

            SearchCallbackHandler _cb;
            shared_ptr<huesdk::BridgeDiscovery> _searcher;
            BridgeSettingsPtr _bridgeSettings;
        };
}  // namespace huestream

#endif  // HUESTREAM_CONNECT_BRIDGESEARCHER_H_
