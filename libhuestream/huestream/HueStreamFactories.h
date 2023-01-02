/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#ifndef HUESTREAM_HUESTREAMFACTORIES_H_
#define HUESTREAM_HUESTREAMFACTORIES_H_

#include "edtls/wrapper/mbedtls/EntropyProviderBase.h"

#include <memory>
#include <string>

#include "support/util/Factory.h"
#include "support/util/MakeUnique.h"
#include "huestream/stream/DtlsEntropyProvider.h"
#include "huestream/connect/IBridgeStorageAccessor.h"
#include "huestream/connect/BridgeFileStorageAccessor.h"
#include "huestream/connect/BasicGroupLightController.h"
#include "huestream/effect/Mixer.h"
#include "huestream/common/language/DummyTranslator.h"
#include "huestream/common/http/IHttpClient.h"
#include "huestream/IHueStream.h"

using BridgeStorageAccessorFactory
    = support::Factory<std::unique_ptr<huestream::IBridgeStorageAccessor>(
            const std::string&, huestream::AppSettingsPtr, huestream::BridgeSettingsPtr)>;

using MixerFactory = support::Factory<std::unique_ptr<huestream::IMixer>(huestream::AppSettingsPtr)>;
using MessageTranslatorFactory = support::Factory<std::unique_ptr<huestream::IMessageTranslator>(std::string)>;
using EntropyProviderFactory = support::Factory<std::unique_ptr<EntropyProviderBase>()>;
using ConnectFactory = support::Factory<std::unique_ptr<huestream::IConnect>(huestream::BridgeHttpClientPtr,
        huestream::MessageDispatcherPtr, huestream::BridgeSettingsPtr,
        huestream::AppSettingsPtr, huestream::StreamPtr, huestream::BridgeStorageAccessorPtr)>;

using HueStreamFactory = support::Factory<std::unique_ptr<huestream::IStream>(huestream::StreamSettingsPtr, huestream::AppSettingsPtr,
        huestream::TimeManagerPtr, huestream::ConnectorPtr)>;
using HttpClientFactory = support::Factory<std::unique_ptr<huestream::IHttpClient>()>;
using ConnectionMonitorFactory
    = support::Factory<std::unique_ptr<huestream::IConnectionMonitor>(huestream::BridgeHttpClientPtr, huestream::AppSettingsPtr)>;

using TimeManagerFactory = support::Factory<std::unique_ptr<huestream::ITimeManager>()>;
using MessageDispatcherFactory = support::Factory<std::unique_ptr<huestream::IMessageDispatcher>()>;
using ConnectorFactory = support::Factory<std::unique_ptr<huestream::IConnector>(huestream::ConfigPtr)>;
using GroupControllerFactory = support::Factory<std::unique_ptr<huestream::IBasicGroupLightController>(huestream::BridgeHttpClientPtr)>;

template<>
std::unique_ptr<EntropyProviderBase> huesdk_lib_default_factory<EntropyProviderBase>();

template<>
std::unique_ptr<huestream::IBridgeStorageAccessor> huesdk_lib_default_factory<huestream::IBridgeStorageAccessor,
                                                                const std::string &,
                                                                huestream::BridgeSettingsPtr>(const std::string &fileName,
                                                                                           huestream::BridgeSettingsPtr hueSettings);


template<>
std::unique_ptr<huestream::IMixer> huesdk_lib_default_factory<huestream::IMixer, huestream::AppSettingsPtr>(huestream::AppSettingsPtr appSettings);

template<>
std::unique_ptr<huestream::IMessageTranslator> huesdk_lib_default_factory<huestream::IMessageTranslator, std::string>(std::string language);

template<>
std::unique_ptr<huestream::IConnect> huesdk_lib_default_factory<huestream::IConnect,
                                                    huestream::BridgeHttpClientPtr,
                                                    huestream::MessageDispatcherPtr,
                                                    huestream::BridgeSettingsPtr,
                                                    huestream::AppSettingsPtr,
                                                    huestream::StreamPtr,
                                                    huestream::BridgeStorageAccessorPtr>(
    huestream::BridgeHttpClientPtr,
    huestream::MessageDispatcherPtr,
    huestream::BridgeSettingsPtr,
    huestream::AppSettingsPtr,
    huestream::StreamPtr,
    huestream::BridgeStorageAccessorPtr);

template<>
std::unique_ptr<huestream::IStream> huesdk_lib_default_factory<huestream::IStream,
                                                huestream::StreamSettingsPtr,
                                                huestream::AppSettingsPtr,
                                                huestream::TimeManagerPtr,
                                                huestream::ConnectorPtr>(
    huestream::StreamSettingsPtr settings,
    huestream::AppSettingsPtr appSettings,
    huestream::TimeManagerPtr timeManager,
    huestream::ConnectorPtr connector);

template<>
std::unique_ptr<huestream::IConnectionMonitor> huesdk_lib_default_factory<huestream::IConnectionMonitor,
                                                           huestream::BridgeHttpClientPtr, huestream::AppSettingsPtr>(
    huestream::BridgeHttpClientPtr httpClient, huestream::AppSettingsPtr appSettings);

template<>
std::unique_ptr<huestream::ITimeManager> huesdk_lib_default_factory<huestream::ITimeManager>();

template<>
std::unique_ptr<huestream::IMessageDispatcher> huesdk_lib_default_factory<huestream::IMessageDispatcher>();

template<>
std::unique_ptr<huestream::IConnector> huesdk_lib_default_factory<huestream::IConnector, huestream::ConfigPtr>(
    huestream::ConfigPtr config);

template<>
std::unique_ptr<huestream::IBasicGroupLightController> huesdk_lib_default_factory<huestream::IBasicGroupLightController,
    huestream::BridgeHttpClientPtr>(huestream::BridgeHttpClientPtr httpClient);


#endif  // HUESTREAM_HUESTREAMFACTORIES_H_
