/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#ifndef HUESTREAM_HUESTREAM_H_
#define HUESTREAM_HUESTREAM_H_

#include <memory>
#include <string>

#include "huestream/IHueStream.h"
#include "huestream/config/Config.h"
#include "huestream/common/data/Bridge.h"
#include "huestream/common/data/Group.h"
#include "huestream/effect/effects/base/Effect.h"
#include "huestream/connect/FeedbackMessage.h"
#include "huestream/common/serialize/SerializerHelper.h"
#include "huestream/common/time/ITimeManager.h"
#include "huestream/common/language/IMessageTranslator.h"
#include "huestream/connect/Connect.h"
#include "huestream/connect/IBridgeStorageAccessor.h"
#include "huestream/connect/IMessageDispatcher.h"
#include "huestream/connect/ConnectionMonitor.h"
#include "huestream/connect/IBasicGroupLightController.h"
#include "huestream/effect/Mixer.h"
#include "huestream/stream/Stream.h"
#include "huestream/connect/ConnectionFlow.h"
#include "huestream/connect/IFeedbackMessageHandler.h"
#include "huestream/effect/lightscript/LightScript.h"
#include "huestream/HueStreamFactories.h"
#include "support/util/Factory.h"
#include "huestream/common/time/TimeProviderProvider.h"

namespace huestream {


/**
 main huestream library facade which aggregates the connect, stream and effect components
 */
class HueStream : public IHueStream {
 public:
    /**
     constructor
     @param config Reference to object containing the library configuration parameters
     */
    explicit HueStream(ConfigPtr config);

    /**
     constructor used for injecting alternative implementations for classes used by HueStream facade
     */
    HueStream(ConfigPtr config,
              HttpClientPtr httpClient);

    /**
     * destructor
     */
    virtual ~HueStream();

    /**
     Get the current config
     */
    ConfigPtr GetConfig() override;


    /**
     set object which handles feedback from the library
     @param handler Reference to object which handles feedback from the library
     */
    void RegisterFeedbackHandler(FeedbackMessageHandlerPtr handler) override;

    /**
     set function which handles feedback from the library
     @param callback Function which handles feedback from the library
     */
    void RegisterFeedbackCallback(FeedbackMessageCallback callback) override;

    /**
     set object which handles light state update events from the library. HueStream will call OnLightStateUpdate(  )
     method of this object *every time* light state of the active group changes.
     Use this method to render the output light states.
     @param handler Reference to object which handles feedback from the library
     */
    void RegisterLightStateUpdatedHandler(LightStateChangedHandlerPtr handler) override;

    /**
     if a valid bridge is stored connect to it, else discover a new bridge and connect to it (blocking execution)
     */
    void ConnectBridge() override;

    /**
     if a valid bridge is stored connect to it, else discover a new bridge and connect to it (non blocking execution)
     @note this is the standard/default connect procedure which should be used for most applications
     */
    void ConnectBridgeAsync() override;

    /**
     if a valid bridge is stored connect to it, else check if a valid bridge is on the network but don't connect yet (blocking execution)
     */
    void ConnectBridgeBackground() override;

    /**
     if a valid bridge is stored connect to it, else check if a valid bridge is on the network but don't connect yet (non blocking execution)
     @note this can used in applications where a Hue system is an optional addition, to notify only users who has a valid Hue bridge
     */
    void ConnectBridgeBackgroundAsync() override;

    /**
     connect to a bridge with manually set ip address (blocking execution)
     @param ipAddress Ip address of the bridge to connect to
     @param useSSL whether or not to use ssl
     */
    void ConnectBridgeManualIp(const std::string &ipAddress) override;

    /**
     connect to a bridge with manually set ip address (non blocking execution)
     @note this is normally used only as a backup when the normal connect call is unable to discover a bridge
     @param ipAddress Ip address of the bridge to connect to
     @param useSSL whether or not to use ssl
     */
    void ConnectBridgeManualIpAsync(const std::string &ipAddress) override;

    /**
     connect to a bridge with manually set ip address, username and clientkey (blocking execution)
     @param bridge Bridge object with a valid ip address, username and clientkey set
     */
    void ConnectManualBridgeInfo(BridgePtr bridge) override;

    /**
     connect to a bridge with manually set ip address, username and clientkey (non blocking execution)
     @note this can be used to manually switch to another bridge from the loaded list of known bridges
     @param bridge Bridge object with a valid ip address, username and clientkey set
     */
    void ConnectManualBridgeInfoAsync(BridgePtr bridge) override;

    /**
     connect to a bridge with manually set id, username and clientkey (blocking execution)
     @note this can be used to manually switch to another bridge with known credentials but unknown ip.
     @param id Id of the bridge to connect to
     @param user Bridge user to use
     @param clientKey Client key associated with this user
     */
    void ConnectBridgeManualIdKey(const std::string& id, const std::string& user, const std::string& clientKey) override;

    /**
     connect to a bridge with manually set id, username and clientkey (non blocking execution)
     @note this can be used to manually switch to another bridge with known credentials but unknown ip.
     @param id Id of the bridge to connect to
     @param user Bridge user to use
     @param clientKey Client key associated with this user
     */
    void ConnectBridgeManualIdKeyAsync(const std::string& id, const std::string& user, const std::string& clientKey) override;

    /**
     connect to a new bridge, even if a valid bridge is already active (blocking execution)
     */
    void ConnectNewBridge() override;

    /**
     connect to a new bridge, even if a valid bridge is already active (non blocking execution)
     @note optional for the case when the user already has an ative bridge connection but explicitely wants to discover a new one
     */
    void ConnectNewBridgeAsync() override;

    /**
     select entertainment group to use (blocking execution)
     @param group Entertainment group to use
     */
    void SelectGroup(GroupPtr group) override;

    /**
     select entertainment group to use (non blocking execution)
     @param group Entertainment group to use
     */
    void SelectGroupAsync(GroupPtr group) override;

    /**
     select entertainment group to use by id (blocking execution)
     @param id Id of the entertainment group to use
     */
    void SelectGroup(std::string id) override;

    /**
     select entertainment group to use by id (non blocking execution)
     @param id Id of the entertainment group to use
     */
    void SelectGroupAsync(std::string id) override;

    /**
     start streaming (blocking execution)
     */
    void Start() override;

    /**
     start streaming (non blocking execution)
     */
    void StartAsync() override;

    /**
     delete stored information of current bridge (blocking execution)
     */
    void ResetBridgeInfo() override;

    /**
     delete stored information of current bridge (non blocking execution)
     */
    void ResetBridgeInfoAsync() override;

    /**
     delete all stored bridge information (blocking execution)
     */
    void ResetAllPersistentData() override;

    /**
     delete all stored bridge information (non blocking execution)
     */
    void ResetAllPersistentDataAsync() override;

    /**
     load stored bridge information (blocking execution)
     */
    void LoadBridgeInfo() override;

    /**
     load stored bridge information (non blocking execution)
     */
    void LoadBridgeInfoAsync() override;

    /**
     abort connection procedure (blocking execution)
     */
    void AbortConnecting() override;

    /**
     abort connection procedure (non blocking execution)
     */
    void AbortConnectingAsync() override;

    /**
     get the result of the last connection procedure
     @return result of the last connection procedure
     */
    ConnectResult GetConnectionResult() override;

    /**
     check if the currently loaded bridged is correctly configured for streaming
     @return whether a bridge is loaded which is correctly configured for streaming
     */
    bool IsStreamableBridgeLoaded() override;

    /**
     check if the currently loaded bridged is being streamed to
     @return whether if the currently loaded bridged is being streamed to
     */
    bool IsBridgeStreaming() override;

    /**
     get the currently loaded bridge
     @return reference to currently loaded bridge object
     */
    BridgePtr GetLoadedBridge() override;

    /**
     get the status of the currently loaded bridge
     @return status of the currently loaded bridge
     */
    BridgeStatus GetLoadedBridgeStatus() override;

    /**
     get the list of entertainment groups on the currently loaded bridge
     @return list of entertainment groups on the currently loaded bridge
     */
    GroupListPtr GetLoadedBridgeGroups() override;

    /**
     get the list of all loaded known bridges that have been authorized for streaming
     @return list of all loaded known bridges that have been authorized for streaming
     */
    BridgeListPtr GetKnownBridges() override;

    /**
     render a single frame
     @note only required if useRenderThread option is disabled
     */
    void RenderSingleFrame() override;

    /**
     stop streaming (blocking execution)
     */
    void Stop() override;

    /**
     stop streaming (non blocking execution)
     */
    void StopAsync() override;

    /**
     lock the engine to be able to add and/or modify effects
     @note required if useRenderThread option is enabled
     */
    void LockMixer() override;

    /**
     unlock the engine for rendering
     @note required if useRenderThread option is enabled
     */
    void UnlockMixer() override;

    /**
     add a single effect to the engine
     @param newEffect Reference to effect to be added
     */
    void AddEffect(EffectPtr newEffect) override;

    /**
     add a full light script to the engine
     @param script Reference to light script to be added
     */
    void AddLightScript(LightScriptPtr script) override;

    /**
     get effect by name
     @param name Name of the effect to retrieve
     @return Reference to effect if found, else nullptr
     */
    EffectPtr GetEffectByName(const std::string &name) override;

    /**
     shut down the huestream module
     */
    void ShutDown() override;

    /**
     prepare the render engine for streaming
     @note mostly internal use: is automatically called as part of starting the stream
     */
    void PrepareMixer() override;


    /**
     Get the number of frames sent
     @note mostly internal use
     */
    int32_t GetStreamCounter() override;

    /**
     Change the streaming mode
     @note mostly internal use
     */
    void ChangeStreamingMode(StreamingMode mode) override;

    /**
    Get the effect mixer
    */
    MixerPtr GetMixer() override;
    /**
     Set the on state of selected entertainment group
     @note this only works when not streaming and a valid group is selected
     */
    void SetGroupOn(bool on) override;

    /**
     Set the brightness of selected entertainment group
     @param brightness Relative brightness between 0 and 1
     @note this only works when not streaming and a valid group is selected
     @note do not update value more than twice per second
     */
    void SetGroupBrightness(double brightness) override;

    /**
     Set the color of selected entertainment group
     @param x X-coordinate in CIE XY color space
     @param y Y-coordinate in CIE XY color space
     @note this only works when not streaming and a valid group is selected
     @note do not update value more than twice per second
     */
    void SetGroupColor(double x, double y) override;

    /**
     Set a default light preset on the selected entertainment group
     @param preset One of the Hue specified presets
     @note this only works when not streaming and a valid group is selected
     */
    void SetGroupPreset(BasicGroupLightController::LightPreset preset, bool excludeLightsWhichAreOff = false) override;

    /**
     Set a custom light preset on the selected entertainment group
     @param brightness Relative brightness between 0 and 1
     @param x X-coordinate in CIE XY color space
     @param y Y-coordinate in CIE XY color space
     @note this only works when not streaming and a valid group is selected
     */
    void SetGroupPreset(double brightness, double x, double y, bool excludeLightsWhichAreOff = false) override;

    /**
     Set the scene of selected entertainment group
     @note this only works when not streaming and a valid group is selected
     */
    void SetGroupScene(const std::string &sceneTag) override;

    /**
     Get the time manager
    */
    TimeManagerPtr GetTimeManager() override;

 protected:
    HueStream(ConfigPtr config,
              HttpClientPtr httpClient,
              TimeManagerPtr timeManager,
              ConnectionMonitorPtr connectionMonitor,
              MixerPtr engine,
              BridgeStorageAccessorPtr storageAccessor,
              MessageDispatcherPtr dispatcher,
              MessageTranslatorPtr translator,
              ConnectorPtr connector,
              BasicGroupLightControllerPtr groupController);

    HueStream(ConfigPtr config,
              HttpClientPtr httpClient,
              TimeManagerPtr timeManager,
              ConnectionMonitorPtr connectionMonitor,
              MixerPtr engine,
              BridgeStorageAccessorPtr storageAccessor,
              MessageDispatcherPtr dispatcher,
              MessageTranslatorPtr translator,
              ConnectorPtr connector,
              BasicGroupLightControllerPtr groupController,
              StreamPtr stream);

    HueStream(ConfigPtr config,
              HttpClientPtr httpClient,
              TimeManagerPtr timeManager,
              ConnectionMonitorPtr connectionMonitor,
              MixerPtr engine,
              BridgeStorageAccessorPtr storageAccessor,
              MessageDispatcherPtr dispatcher,
              MessageTranslatorPtr translator,
              ConnectorPtr connector,
              BasicGroupLightControllerPtr groupController,
              StreamPtr stream,
              ConnectPtr hueConnect);

    virtual void NewFeedbackMessage(const FeedbackMessage &message);
    virtual void Render();

    ConfigPtr _config;
    FeedbackMessageHandlerPtr _handler;
    FeedbackMessageCallback _callback;
    ConnectionMonitorPtr _connectionMonitor;
    BridgeListPtr _knownBridges;
    BasicGroupLightControllerPtr _groupController;

 PROP_DEFINE(HueStream, BridgePtr, activeBridge, ActiveBridge);
 PROP_DEFINE(HueStream, StreamRenderCallback, streamRenderCallback, StreamRenderCallback);

 private:
    BridgeStorageAccessorPtr _storageAccessor;
    MessageDispatcherPtr _dispatcher;
    MessageTranslatorPtr _translator;
    LightStateChangedHandlerPtr _lightStateChangedHandler;
    TimeManagerPtr _timeManager;
    ConnectorPtr _connector;
    StreamPtr _stream;
    ConnectPtr _connect;
    MixerPtr _mixer;
    ScopedTimeProviderProvider _timeProvider;
};

}  // namespace huestream

#endif  // HUESTREAM_HUESTREAM_H_
