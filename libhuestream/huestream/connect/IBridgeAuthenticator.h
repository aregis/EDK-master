/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#ifndef HUESTREAM_CONNECT_IBRIDGEAUTHENTICATOR_H_
#define HUESTREAM_CONNECT_IBRIDGEAUTHENTICATOR_H_

#include "huestream/common/data/Bridge.h"
#include "huestream/config/AppSettings.h"

#include <memory>

namespace huestream {

        using AuthenticateCallbackHandler = std::function<void(BridgePtr)>;

        class IBridgeAuthenticator {
        public:
            virtual ~IBridgeAuthenticator() = default;

            virtual void Authenticate(BridgePtr bridge, AppSettingsPtr appSetting, AuthenticateCallbackHandler cb) = 0;

        protected:
        };

        using BridgeAuthenticatorPtr = std::shared_ptr<IBridgeAuthenticator>;

}  // namespace huestream

#endif  // HUESTREAM_CONNECT_IBRIDGEAUTHENTICATOR_H_
