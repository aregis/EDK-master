include_directories(
        ..
)

list(APPEND huestream_files
    HueEDK.cpp
    HueStream.cpp
    HueStreamFactories.cpp
    common/data/ApiVersion.cpp
    common/data/Area.cpp
    common/data/Bridge.cpp
    common/data/BridgeSettings.cpp
    common/data/Color.cpp
    common/data/CuboidArea.cpp
    common/data/Group.cpp
    common/data/Zone.cpp
    common/data/HueStreamData.cpp
    common/data/Light.cpp
    common/data/Location.cpp
    common/data/Scene.cpp
    common/http/BridgeHttpClient.cpp
    common/http/HttpClient.cpp
    common/http/HttpClientProvider.cpp
    common/http/HttpRequestInfo.cpp
    common/language/DummyTranslator.cpp
    common/serialize/ObjectBuilderBase.cpp
    common/serialize/Serializable.cpp
    common/storage/FileStorageAccessor.cpp
    common/time/TimeManager.cpp
    common/util/HueMath.cpp
    common/util/Rand.cpp
    config/AppSettings.cpp
    config/Config.cpp
    config/ObjectBuilder.cpp
    connect/Authenticator.cpp
    connect/BasicGroupLightController.cpp
    connect/BridgeFileStorageAccessor.cpp
    connect/BridgeSearcher.cpp
    connect/BridgeStreamingChecker.cpp
    connect/Connect.cpp
    connect/ConnectionFlow.cpp
    connect/ConnectionFlowFactory.cpp
    connect/ConnectionMonitor.cpp
    connect/FeedbackMessage.cpp
    connect/FullConfigRetriever.cpp
    connect/MessageDispatcher.cpp
    connect/BridgeConfigRetriever.cpp
    effect/Mixer.cpp
    effect/animation/ActionPlayer.cpp
    effect/animation/Player.cpp
    effect/animation/animations/ConstantAnimation.cpp
    effect/animation/animations/CurveAnimation.cpp
    effect/animation/animations/RandomAnimation.cpp
    effect/animation/animations/SequenceAnimation.cpp
    effect/animation/animations/TweenAnimation.cpp
    effect/animation/animations/FramesAnimation.cpp
    effect/animation/animations/base/Animation.cpp
    effect/animation/animations/base/AnimationHelper.cpp
    effect/animation/animations/base/RepeatableAnimation.cpp
    effect/animation/animations/base/TriggerableAnimation.cpp
    effect/animation/data/Channel.cpp
    effect/animation/data/CurveData.cpp
    effect/animation/data/CurveOptions.cpp
    effect/animation/data/Point.cpp
    effect/animation/data/PointHelper.cpp
    effect/animation/data/TweenType.cpp
    effect/animation/data/Vector.cpp
    effect/effects/AreaEffect.cpp
    effect/effects/ExplosionEffect.cpp
    effect/effects/HitEffect.cpp
    effect/effects/LightIteratorEffect.cpp
    effect/effects/LightSourceEffect.cpp
    effect/effects/SphereLightSourceEffect.cpp
    effect/effects/ManualEffect.cpp
    effect/effects/MultiChannelEffect.cpp
    effect/effects/SequenceEffect.cpp
    effect/effects/base/AnimationEffect.cpp
    effect/effects/base/ColorAnimationEffect.cpp
    effect/effects/base/Effect.cpp
    effect/effects/base/RadialEffect.cpp
    effect/lightscript/Action.cpp
    effect/lightscript/LightScript.cpp
    effect/lightscript/Timeline.cpp
    stream/DtlsConnector.cpp
    stream/DtlsEntropyProvider.cpp
    stream/DtlsTimerProvider.cpp
    stream/DtlsUdpClient.cpp
    stream/ProtocolSerializer.cpp
    stream/Stream.cpp
    stream/StreamFactory.cpp
    stream/StreamSettings.cpp
    stream/StreamStarter.cpp
    stream/UdpConnector.cpp
    HueEDK.h
    HueStream.h
    HueStreamFactories.h
    IHueStream.h
    common/data/ApiVersion.h
    common/data/Area.h
    common/data/Bridge.h
    common/data/BridgeSettings.h
    common/data/Color.h
    common/data/CuboidArea.h
    common/data/Group.h
    common/data/HueStreamData.h
    common/data/IArea.h
    common/data/Light.h
    common/data/Location.h
    common/data/Scene.h
    common/data/Zone.h
    common/http/BridgeHttpClient.h
    common/http/HttpClient.h
    common/http/HttpClientProvider.h
    common/http/HttpRequestInfo.h
    common/http/IBridgeHttpClient.h
    common/http/IHttpClient.h
    common/language/DummyTranslator.h
    common/language/IMessageTranslator.h
    common/serialize/ObjectBuilderBase.h
    common/serialize/Serializable.h
    common/serialize/SerializerHelper.h
    common/storage/FileStorageAccessor.h
    common/storage/IStorageAccessor.h
    common/time/ITimeManager.h
    common/time/ITimeProvider.h
    common/time/TimeManager.h
    common/time/TimeProviderProvider.h
    common/util/HueMath.h
    common/util/Rand.h
    config/AppSettings.h
    config/Config.h
    config/ObjectBuilder.h
    connect/Authenticator.h
    connect/BasicGroupLightController.h
    connect/BridgeFileStorageAccessor.h
    connect/BridgeSearcher.h
    connect/BridgeStreamingChecker.h
    connect/Connect.h
    connect/ConnectionFlow.h
    connect/ConnectionFlowFactory.h
    connect/ConnectionMonitor.h
    connect/FeedbackMessage.h
    connect/FullConfigRetriever.h
    connect/IBasicGroupLightController.h
    connect/IBridgeAuthenticator.h
    connect/IBridgeSearcher.h
    connect/IBridgeStorageAccessor.h
    connect/IBridgeStreamingChecker.h
    connect/IConnectionFlow.h
    connect/IConnectionFlowFactory.h
    connect/IConnectionMonitor.h
    connect/IFeedbackMessageHandler.h
    connect/IFullConfigRetriever.h
    connect/IMessageDispatcher.h
    connect/MessageDispatcher.h
    connect/OperationResult.h
    connect/BridgeConfigRetriever.h
    effect/IMixer.h
    effect/Mixer.h
    effect/animation/ActionPlayer.h
    effect/animation/IPlayer.h
    effect/animation/Player.h
    effect/animation/animations/ConstantAnimation.h
    effect/animation/animations/CurveAnimation.h
    effect/animation/animations/RandomAnimation.h
    effect/animation/animations/SequenceAnimation.h
    effect/animation/animations/TweenAnimation.h
    effect/animation/animations/FramesAnimation.h
    effect/animation/animations/base/Animation.h
    effect/animation/animations/base/AnimationHelper.h
    effect/animation/animations/base/RepeatableAnimation.h
    effect/animation/animations/base/TriggerableAnimation.h
    effect/animation/data/Channel.h
    effect/animation/data/CurveData.h
    effect/animation/data/CurveOptions.h
    effect/animation/data/Point.h
    effect/animation/data/PointHelper.h
    effect/animation/data/TweenType.h
    effect/animation/data/Vector.h
    effect/effects/AreaEffect.h
    effect/effects/ExplosionEffect.h
    effect/effects/HitEffect.h
    effect/effects/LightIteratorEffect.h
    effect/effects/LightSourceEffect.h
    effect/effects/SphereLightSourceEffect.h
    effect/effects/ManualEffect.h
    effect/effects/MultiChannelEffect.h
    effect/effects/SequenceEffect.h
    effect/effects/base/AnimationEffect.h
    effect/effects/base/ColorAnimationEffect.h
    effect/effects/base/Effect.h
    effect/effects/base/RadialEffect.h
    effect/lightscript/Action.h
    effect/lightscript/ITimeline.h
    effect/lightscript/LightScript.h
    effect/lightscript/Timeline.h
    stream/DtlsConnector.h
    stream/DtlsEntropyProvider.h
    stream/DtlsTimerProvider.h
    stream/DtlsUdpClient.h
    stream/IConnector.h
    stream/IStream.h
    stream/IStreamFactory.h
    stream/IStreamStarter.h
    stream/ProtocolSerializer.h
    stream/Stream.h
    stream/StreamFactory.h
    stream/StreamSettings.h
    stream/StreamStarter.h
    stream/UdpConnector.h
)

add_library(huestream ${LIB_BUILD_MODE} ${huestream_files})
target_include_directories(huestream INTERFACE "..")

set(LIBS bridge_discovery support support_mock json edtls_client mbedcl_wrapper mbedtls mbedcrypto mbedx509 nghttp2_static)

if (WIN32)
    set(LIBS ${LIBS} Iphlpapi ws2_32)
endif()

if (ANDROID)
    set(LIBS ${LIBS} atomic)
endif()

#---------------------------------------------
# Let's build a framework on iOS
#---------------------------------------------
if (IOS)
    set_target_properties(huestream PROPERTIES
        FRAMEWORK TRUE
        #PUBLIC_HEADER "${huestream_public_headers}"
        MACOSX_FRAMEWORK_IDENTIFIER com.lighting.huestream
        MACOSX_PACKAGE_LOCATION Headers
    )

    foreach(elem IN ZIP_LISTS huestream_files)
        string(REGEX MATCH "(.*\/)?(.*\\.h)" output "${elem_0}")
        string(LENGTH "${CMAKE_MATCH_1}" var_len)
        math(EXPR last_char_index "${var_len} - 1")
        string(SUBSTRING "${CMAKE_MATCH_1}" 0 ${last_char_index} dir_path)
        if(${elem_0} STREQUAL "${output}")
           SET_PROPERTY(SOURCE ${elem_0} PROPERTY MACOSX_PACKAGE_LOCATION Headers/huestream/${dir_path})
        endif()
    endforeach()

    add_custom_command(TARGET huestream POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_BINARY_DIR}/external_install/include ${PROJECT_BINARY_DIR}/bin/huestream.framework/Headers)
    add_custom_command(TARGET huestream POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/../support/include ${PROJECT_BINARY_DIR}/bin/huestream.framework/Headers)
    add_custom_command(TARGET huestream POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/../bridgediscovery/include ${PROJECT_BINARY_DIR}/bin/huestream.framework/Headers)

endif()

add_library_dependencies(huestream ${LIBS})

install(TARGETS huestream
        RUNTIME DESTINATION ${INSTALL_DIR}
        LIBRARY DESTINATION ${INSTALL_DIR}
        ARCHIVE DESTINATION ${INSTALL_DIR}
        FRAMEWORK DESTINATION ${INSTALL_DIR})

install(DIRECTORY ${CMAKE_SOURCE_DIR}/libhuestream/huestream DESTINATION ${INSTALL_DIR}/include
        FILES_MATCHING PATTERN "*.h")
