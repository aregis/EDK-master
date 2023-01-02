/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#ifndef HUESTREAM_STREAM_DTLSCONNECTOR_H_
#define HUESTREAM_STREAM_DTLSCONNECTOR_H_

#include "huestream/stream/IConnector.h"


#include "edtls/client/IClientNotifier.h"
#include "edtls/client/DTLSClient.h"
#include "edtls/wrapper/IClientWrapper.h"
#include "edtls/wrapper/mbedtls/MbedtlsClientPlatform.h"

#include <memory>

typedef void (*LogFunction)(const char *text);

namespace huestream {

    class DtlsConnector : public IConnector, public IClientNotifier {
    private:
        std::shared_ptr<DTLSClient> client_;
        std::shared_ptr<IClientWrapper> wrapper_;
        EntropyProviderBasePtr entropyProvider_;

        void handshakeFailed() override;

        void peer_closed() override;

    public:
        explicit DtlsConnector(EntropyProviderBasePtr entropyProvider, LogFunction logFunction = nullptr);

        virtual ~DtlsConnector();

        bool Connect(BridgePtr bridge, unsigned short port) override;

        void Disconnect() override;

        bool Send(const char *buffer, unsigned int bufferSize) override;

        MbedtlsClientPlatformPtr CreatePlatform(LogFunction logFunction) const;

        EntropyProviderBasePtr GetEntropyProvider() const;
    };
}  // namespace huestream

#endif  // HUESTREAM_STREAM_DTLSCONNECTOR_H_
