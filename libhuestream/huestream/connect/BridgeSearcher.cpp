/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <huestream/connect/BridgeSearcher.h>

#include <memory>

using std::vector;
using huesdk::BridgeDiscoveryReturnCode;
using huesdk::BridgeDiscoveryResult;
using huesdk::BridgeDiscovery;

namespace huestream {

        void BridgeSearcher::operator()(const vector<std::shared_ptr<BridgeDiscoveryResult>> &results,
                                               BridgeDiscoveryReturnCode return_code) {
            if (!_searcher)
                return;

            auto bridges = std::make_shared<BridgeList>();
            if (return_code == BridgeDiscoveryReturnCode::BRIDGE_DISCOVERY_RETURN_CODE_SUCCESS) {
                for (auto result : results) {
                    auto bridge = std::make_shared<Bridge>(_bridgeSettings);
                    bridge->SetId(result->get_unique_id());
                    bridge->SetIpAddress(result->get_ip());
                    bridge->SetIsValidIp(true);
                    bridge->SetApiversion(result->get_api_version());
                    bridge->SetModelId(result->get_model_id());
                    bridge->SetName(result->get_name());
                    bridge->SetSwversion(result->get_swversion());
                    bridges->push_back(bridge);                    
                }
            }
            _cb(bridges);
        }

        void BridgeSearcher::SearchNew(SearchCallbackHandler cb) {
            SearchNew(false, cb);
        }

        void BridgeSearcher::SearchNew(bool bruteForce, SearchCallbackHandler cb) {
            if (_searcher)
                return;
            _cb = cb;
            _searcher = std::make_shared<BridgeDiscovery>();
            if (bruteForce) {
                _searcher->search(BridgeDiscovery::Option::MDNS |
                                  BridgeDiscovery::Option::NUPNP |
                                  BridgeDiscovery::Option::IPSCAN, this);
            } else {
                _searcher->search(this);
            }
        }

        void BridgeSearcher::Abort() {
            if (!_searcher)
                return;
            _searcher->stop();
            _searcher = nullptr;
        }

        BridgeSearcher::BridgeSearcher(BridgeSettingsPtr bridgeSettings) :
        _bridgeSettings(bridgeSettings) {}
}  // namespace huestream
