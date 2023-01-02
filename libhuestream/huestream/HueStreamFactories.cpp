
/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#ifndef HUESDK_HUESTREAMDEFAULTFACTORIES_H
#define HUESDK_HUESTREAMDEFAULTFACTORIES_H

#include <string>
#include <memory>

#include "support/util/Factory.h"
#include "support/util/MakeUnique.h"
#include "huestream/connect/IBridgeStorageAccessor.h"
#include "huestream/connect/BridgeFileStorageAccessor.h"
#include "huestream/connect/BridgeStreamingChecker.h"
#include "huestream/common/language/DummyTranslator.h"
#include "huestream/common/http/HttpClient.h"
#include "huestream/common/http/BridgeHttpClient.h"
#include "huestream/common/time/TimeManager.h"
#include "huestream/effect/Mixer.h"
#include "huestream/stream/DtlsEntropyProvider.h"
#include "huestream/HueStreamFactories.h"
#include "huestream/stream/DtlsConnector.h"
#include "huestream/stream/UdpConnector.h"

template<>
std::unique_ptr<EntropyProviderBase> huesdk_lib_default_factory<EntropyProviderBase>() {
    return support::make_unique<huestream::DtlsEntropyProvider>();
}

template<>
std::unique_ptr<huestream::IBridgeStorageAccessor> huesdk_lib_default_factory<huestream::IBridgeStorageAccessor,
        const std::string &,
        huestream::AppSettingsPtr,
        huestream::BridgeSettingsPtr>(const std::string &fileName,
                                      huestream::AppSettingsPtr appSettings,
                                      huestream::BridgeSettingsPtr bridgeSettings) {
    return support::make_unique<huestream::BridgeFileStorageAccessor>(fileName, appSettings, bridgeSettings);
}

template<>
std::unique_ptr<huestream::IMixer> huesdk_lib_default_factory<huestream::IMixer, huestream::AppSettingsPtr>(huestream::AppSettingsPtr appSettings) {
    return support::make_unique<huestream::Mixer>(appSettings);
}

template<>
std::unique_ptr<huestream::IMessageTranslator> huesdk_lib_default_factory<huestream::IMessageTranslator, std::string>(std::string language) {
    return support::make_unique<huestream::DummyTranslator>(language);
}

template<>
std::unique_ptr<huestream::IConnect> huesdk_lib_default_factory<huestream::IConnect,
                                                    huestream::BridgeHttpClientPtr,
                                                    huestream::MessageDispatcherPtr,
                                                    huestream::BridgeSettingsPtr,
                                                    huestream::AppSettingsPtr,
                                                    huestream::StreamPtr,
                                                    huestream::BridgeStorageAccessorPtr>(
    huestream::BridgeHttpClientPtr httpClient,
    huestream::MessageDispatcherPtr messageDispatcher,
    huestream::BridgeSettingsPtr bridgeSettings,
    huestream::AppSettingsPtr appSettings,
    huestream::StreamPtr stream,
    huestream::BridgeStorageAccessorPtr bridgeStorageAccessor) {

    return support::make_unique<huestream::Connect>(
        httpClient,
        messageDispatcher,
        bridgeSettings,
        appSettings,
        stream,
        bridgeStorageAccessor);
}

template<>
std::unique_ptr<huestream::IStream> huesdk_lib_default_factory<huestream::IStream,
                                                huestream::StreamSettingsPtr,
                                                huestream::AppSettingsPtr,
                                                huestream::TimeManagerPtr,
                                                huestream::ConnectorPtr>(
    huestream::StreamSettingsPtr streamSettings,
    huestream::AppSettingsPtr appSettings,
    huestream::TimeManagerPtr timeManager,
    huestream::ConnectorPtr connector) {
    return support::make_unique<huestream::Stream>(
        streamSettings,
        appSettings,
        timeManager,
        connector);
}

template<>
std::unique_ptr<huestream::IHttpClient> huesdk_lib_default_factory<huestream::IHttpClient>() {
    return support::make_unique<huestream::HttpClient>();
}

template<>
std::unique_ptr<huestream::IConnectionMonitor> huesdk_lib_default_factory<huestream::IConnectionMonitor,
                                                           huestream::BridgeHttpClientPtr, huestream::AppSettingsPtr>(
    huestream::BridgeHttpClientPtr httpClient, huestream::AppSettingsPtr appSettings) {
    return support::make_unique<huestream::ConnectionMonitor>(
        support::make_unique<huestream::BridgeStreamingChecker>(
            support::make_unique<huestream::FullConfigRetriever>(httpClient, appSettings->UseForcedActivation()), httpClient));
}

template<>
std::unique_ptr<huestream::ITimeManager> huesdk_lib_default_factory<huestream::ITimeManager>() {
    return support::make_unique<huestream::TimeManager>();
}

template<>
std::unique_ptr<huestream::IMessageDispatcher> huesdk_lib_default_factory<huestream::IMessageDispatcher>() {
    return support::make_unique<huestream::MessageDispatcher>();
}

template<>
std::unique_ptr<huestream::IConnector> huesdk_lib_default_factory<huestream::IConnector, huestream::ConfigPtr>(
    huestream::ConfigPtr config) {
    if (config->GetStreamingMode() == huestream::STREAMING_MODE_UDP) {
        return support::make_unique<huestream::UdpConnector>();
    }

    return support::make_unique<huestream::DtlsConnector>(EntropyProviderFactory::create());
}

template<>
std::unique_ptr<huestream::IBasicGroupLightController> huesdk_lib_default_factory<huestream::IBasicGroupLightController,
    huestream::BridgeHttpClientPtr>(
        huestream::BridgeHttpClientPtr httpClient) {
    return support::make_unique<huestream::BasicGroupLightController>(httpClient);
}

#endif  // HUESDK_HUESTREAMDEFAULTFACTORIES_H
