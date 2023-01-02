/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#ifndef HUESTREAM_CONNECT_AUTHENTICATOR_H_
#define HUESTREAM_CONNECT_AUTHENTICATOR_H_

#include "huestream/common/http/IBridgeHttpClient.h"
#include "huestream/connect/ConnectionFlow.h"

namespace huestream {

        class Authenticator : public IBridgeAuthenticator {
        public:
            explicit Authenticator(BridgeHttpClientPtr http);

            void Authenticate(BridgePtr bridge, AppSettingsPtr appSettings, AuthenticateCallbackHandler cb) override;

            void Abort();

        protected:
            BridgeHttpClientPtr _http;

            std::string CreateDeviceType(AppSettingsPtr appSettings);

            void ParseCredentials(JSONNode root, BridgePtr bridge);

            void CheckClientkeyAvailable(JSONNode root, BridgePtr bridge);
        };

}  // namespace huestream

#endif  // HUESTREAM_CONNECT_AUTHENTICATOR_H_
