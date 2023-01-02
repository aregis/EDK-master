/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <huestream/stream/DtlsConnector.h>
#include <huestream/stream/DtlsEntropyProvider.h>
#include <huestream/stream/DtlsTimerProvider.h>
#include <huestream/stream/DtlsUdpClient.h>
#include <huestream/config/Config.h>

#include <edtls/wrapper/mbedtls/MbedtlsClientWrapperFactory.h>

#include <vector>
#include <string>
#include <memory>

#ifdef _WINDOWS
#include <windows.h>
#include <codecvt>
#endif

namespace huestream {

static void DefaultPrintfLogger(const char *s) {
    printf("%s", s);

#ifdef _WINDOWS
		if (IsDebuggerPresent()) {
			OutputDebugStringW(std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>>().from_bytes(s).c_str());
		}
#endif
}

void DtlsConnector::handshakeFailed() {
    printf("HANDSHAKE FAILED!\n");
}

void DtlsConnector::peer_closed() {
}

std::vector<unsigned char> HexToBytes(const std::string &hex) {
    std::vector<unsigned char> bytes;

    for (unsigned int i = 0; i < hex.length(); i += 2) {
        std::string byteString = hex.substr(i, 2);
        unsigned char byte = static_cast<unsigned char>(strtol(byteString.c_str(), NULL, 16));
        bytes.push_back(byte);
    }

    return bytes;
}

bool DtlsConnector::Connect(BridgePtr bridge, unsigned short port) {
    auto strPort = std::to_string(port);
    auto pskInfo = PSKInfo(bridge->IsSupportingClipV2() ? bridge->GetAppId().c_str() : bridge->GetUser().c_str(), HexToBytes(bridge->GetClientKey()));
    return client_->connect(bridge->GetIpAddress().c_str(), strPort.c_str(), pskInfo);
}

void DtlsConnector::Disconnect() {
    client_->close();
}

bool DtlsConnector::Send(const char *buffer, unsigned int bufferSize) {
    client_->send(const_cast<char *>(buffer), bufferSize);
    // todo: check return value
    return true;
}

DtlsConnector::DtlsConnector(EntropyProviderBasePtr entropyProvider, LogFunction logFunction) {
    entropyProvider_ = entropyProvider;
    wrapper_ = MbedtlsClientWrapperFactory::get(CreatePlatform(logFunction));
    client_ = std::make_shared<DTLSClient>(wrapper_, this);
}

DtlsConnector::~DtlsConnector() {
    client_->close();
}

MbedtlsClientPlatformPtr DtlsConnector::CreatePlatform(LogFunction logFunction) const {
    return std::make_shared<MbedtlsClientPlatform>(
        logFunction != nullptr ? logFunction : DefaultPrintfLogger,
        entropyProvider_,
        std::make_shared<DtlsTimerProvider>(),
        std::make_shared<DtlsUdpClient>());
}
EntropyProviderBasePtr DtlsConnector::GetEntropyProvider() const {
    return entropyProvider_;
}
}  // namespace huestream
