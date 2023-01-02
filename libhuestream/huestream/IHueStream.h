/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#ifndef HUESTREAM_IHUESTREAM_H_
#define HUESTREAM_IHUESTREAM_H_

#include <memory>
#include <string>

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
#include "huestream/effect/Mixer.h"
#include "huestream/stream/Stream.h"
#include "huestream/connect/ConnectionFlow.h"
#include "huestream/connect/IFeedbackMessageHandler.h"
#include "huestream/effect/lightscript/LightScript.h"
#include "huestream/HueStreamFactories.h"
#include "support/util/Factory.h"

namespace huestream {

typedef std::function<void(const FeedbackMessage &message)> FeedbackMessageCallback;

class Config;
typedef std::shared_ptr<Config> ConfigPtr;

class IConnect;
typedef std::shared_ptr<IConnect> ConnectPtr;

class IHueStream;
typedef shared_ptr<IHueStream> HueStreamPtr;

class ILightStateChangedHandler {
 public:
    ILightStateChangedHandler() = default;
    virtual ~ILightStateChangedHandler() = default;

    virtual void OnLightStateUpdated(LightListPtr lights) = 0;
};

typedef std::shared_ptr<ILightStateChangedHandler> LightStateChangedHandlerPtr;
typedef std::shared_ptr<IFeedbackMessageHandler> FeedbackMessageHandlerPtr;

class IHueStream {
 public:
    virtual ~IHueStream() = default;

    /**
     Get the current config
     */
    virtual ConfigPtr GetConfig() = 0;

    /**
     set object which handles feedback from the library
     @param handler Reference to object which handles feedback from the library
     */
    virtual void RegisterFeedbackHandler(FeedbackMessageHandlerPtr handler) = 0;

    /**
     set function which handles feedback from the library
     @param callback Function which handles feedback from the library
     */
    virtual void RegisterFeedbackCallback(FeedbackMessageCallback callback) = 0;

    /**
     set object which handles light state update events from the library. HueStream will call OnLightStateUpdate(  )
     method of this object *every time* light state of the active group changes.
     Use this method to render the output light states.
     @param handler Reference to object which handles feedback from the library
     */
    virtual void RegisterLightStateUpdatedHandler(LightStateChangedHandlerPtr handler) = 0;

    /**
     if a valid bridge is stored connect to it, else discover a new bridge and connect to it (blocking execution)
     */
    virtual void ConnectBridge() = 0;

    /**
     if a valid bridge is stored connect to it, else discover a new bridge and connect to it (non blocking execution)
     @note this is the standard/default connect procedure which should be used for most applications
     */
    virtual void ConnectBridgeAsync() = 0;

    /**
     if a valid bridge is stored connect to it, else check if a valid bridge is on the network but don't connect yet (blocking execution)
     */
    virtual void ConnectBridgeBackground() = 0;

    /**
     if a valid bridge is stored connect to it, else check if a valid bridge is on the network but don't connect yet (non blocking execution)
     @note this can used in applications where a Hue system is an optional addition, to notify only users who has a valid Hue bridge
     */
    virtual void ConnectBridgeBackgroundAsync() = 0;

    /**
     connect to a bridge with manually set ip address (blocking execution)
     @param ipAddress Ip address of the bridge to connect to
     */
    virtual void ConnectBridgeManualIp(const std::string &ipAddress) = 0;

    /**
     connect to a bridge with manually set ip address (non blocking execution)
     @note this is normally used only as a backup when the normal connect call is unable to discover a bridge
     @param ipAddress Ip address of the bridge to connect to
     */
    virtual void ConnectBridgeManualIpAsync(const std::string &ipAddress) = 0;

    /**
     connect to a bridge with manually set ip address, username and clientkey (blocking execution)
     @param bridge Bridge object with a valid ip address, username and clientkey set
     */
    virtual void ConnectManualBridgeInfo(BridgePtr bridge)  = 0;

    /**
     connect to a bridge with manually set ip address, username and clientkey (non blocking execution)
     @note this can be used to manually switch to another bridge from the loaded list of known bridges
     @param bridge Bridge object with a valid ip address, username and clientkey set
     */
    virtual void ConnectManualBridgeInfoAsync(BridgePtr bridge) = 0;

    /**
    connect to a bridge with manually set id, username and clientkey (blocking execution)
    @note this can be used to manually switch to another bridge with known credentials but unknown ip.
    @param id Id of the bridge to connect to
    @param user Bridge user to use
    @param clientKey Client key associated with this user
    */
    virtual void ConnectBridgeManualIdKey(const std::string& id, const std::string& user, const std::string& clientKey) = 0;

    /**
     connect to a bridge with manually set id, username and clientkey (non blocking execution)
     @note this can be used to manually switch to another bridge with known credentials but unknown ip.
     @param id Id of the bridge to connect to
    @param user Bridge user to use
    @param clientKey Client key associated with this user
     */
    virtual void ConnectBridgeManualIdKeyAsync(const std::string& id, const std::string& user, const std::string& clientKey) = 0;

    /**
     connect to a new bridge, even if a valid bridge is already active (blocking execution)
     */
    virtual void ConnectNewBridge() = 0;

    /**
     connect to a new bridge, even if a valid bridge is already active (non blocking execution)
     @note optional for the case when the user already has an ative bridge connection but explicitely wants to discover a new one
     */
    virtual void ConnectNewBridgeAsync() = 0;
    /**
     select entertainment group to use (blocking execution)
     @param group Entertainment group to use
     */
    virtual void SelectGroup(GroupPtr group) = 0;

    /**
     select entertainment group to use (non blocking execution)
     @param group Entertainment group to use
     */
    virtual void SelectGroupAsync(GroupPtr group) = 0;

    /**
     select entertainment group to use by id (blocking execution)
     @param id Id of the entertainment group to use
     */
    virtual void SelectGroup(std::string id) = 0;

    /**
     select entertainment group to use by id (non blocking execution)
     @param id Id of the entertainment group to use
     */
    virtual void SelectGroupAsync(std::string id) = 0;

    /**
     start streaming (blocking execution)
     */
    virtual void Start() = 0;

    /**
     start streaming (non blocking execution)
     */
    virtual void StartAsync() = 0;

    /**
     delete stored bridge information of active bridge (blocking execution)
     */
    virtual void ResetBridgeInfo() = 0;

    /**
     delete stored bridge information of active bridge (non blocking execution)
     */
    virtual void ResetBridgeInfoAsync() = 0;

    /**
     delete all stored bridge information (blocking execution)
     */
    virtual void ResetAllPersistentData() = 0;

    /**
     delete all stored bridge information (non blocking execution)
     */
    virtual void ResetAllPersistentDataAsync() = 0;

    /**
     load stored bridge information (blocking execution)
     */
    virtual void LoadBridgeInfo() = 0;

    /**
     load stored bridge information (non blocking execution)
     */
    virtual void LoadBridgeInfoAsync() = 0;

    /**
     abort connection procedure (blocking execution)
     */
    virtual void AbortConnecting() = 0;

    /**
     abort connection procedure (non blocking execution)
     */
    virtual void AbortConnectingAsync() = 0;

    /**
     get the result of the last connection procedure
     @return result of the last connection procedure
     */
    virtual ConnectResult GetConnectionResult() = 0;

    /**
     check if the currently loaded bridged is correctly configured for streaming
     @return whether a bridge is loaded which is correctly configured for streaming
     */
    virtual bool IsStreamableBridgeLoaded() = 0;

    /**
     check if the currently loaded bridged is being streamed to
     @return whether if the currently loaded bridged is being streamed to
     */
    virtual bool IsBridgeStreaming() = 0;

    /**
     get the currently loaded bridge
     @return reference to currently loaded bridge object
     */
    virtual BridgePtr GetLoadedBridge() = 0;

    /**
     get the status of the currently loaded bridge
     @return status of the currently loaded bridge
     */
    virtual BridgeStatus GetLoadedBridgeStatus() = 0;

    /**
     get the list of entertainment groups on the currently loaded bridge
     @return list of entertainment groups on the currently loaded bridge
     */
    virtual GroupListPtr GetLoadedBridgeGroups() = 0;

    /**
     get the list of all loaded known bridges that have been authorized for streaming
     @return list of all loaded known bridges that have been authorized for streaming
     */
    virtual BridgeListPtr GetKnownBridges() = 0;

    /**
     render a single frame
     @note only required if useRenderThread option is disabled
     */
    virtual void RenderSingleFrame() = 0;

    /**
     stop streaming (blocking execution)
     */
    virtual void Stop() = 0;

    /**
     stop streaming (non blocking execution)
     */
    virtual void StopAsync() = 0;

    /**
     lock the engine to be able to add and/or modify effects
     @note required if useRenderThread option is enabled
     */
    virtual void LockMixer() = 0;

    /**
     unlock the engine for rendering
     @note required if useRenderThread option is enabled
     */
    virtual void UnlockMixer() = 0;

    /**
     add a single effect to the engine
     @param newEffect Reference to effect to be added
     */
    virtual void AddEffect(EffectPtr newEffect) = 0;

    /**
     add a full light script to the engine
     @param script Reference to light script to be added
     */
    virtual void AddLightScript(LightScriptPtr script) = 0;

    /**
     get effect by name
     @param name Name of the effect to retrieve
     @return Reference to effect if found, else nullptr
     */
    virtual EffectPtr GetEffectByName(const std::string &name) = 0;

    /**
     shut down the huestream module
     */
    virtual void ShutDown() = 0;

    /**
     prepare the render engine for streaming
     @note mostly internal use: is automatically called as part of starting the stream
     */
    virtual void PrepareMixer() = 0;


    /**
     Get the number of frames sent
     @note mostly internal use
     */
    virtual int32_t GetStreamCounter() = 0;

    /**
     Change the streaming mode
     @note mostly internal use
     */
    virtual void ChangeStreamingMode(StreamingMode mode) = 0;

    /**
     Get the effect mixer
     */
    virtual MixerPtr GetMixer() = 0;

    /**
     Set the on state of selected entertainment group
     @note this only works when not streaming and a valid group is selected
     */
    virtual void SetGroupOn(bool on) = 0;

    /**
     Set the brightness of selected entertainment group
     @param brightness Relative brightness between 0 and 1
     @note this only works when not streaming and a valid group is selected
     @note do not update value more than twice per second
     */
    virtual void SetGroupBrightness(double brightness) = 0;

    /**
     Set the color of selected entertainment group
     @param x X-coordinate in CIE XY color space
     @param y Y-coordinate in CIE XY color space
     @note this only works when not streaming and a valid group is selected
     @note do not update value more than twice per second
     */
    virtual void SetGroupColor(double x, double y) = 0;

    /**
     Set a default light preset on the selected entertainment group
     @param preset One of the Hue specified presets
     @note this only works when not streaming and a valid group is selected
     */
    virtual void SetGroupPreset(BasicGroupLightController::LightPreset preset, bool excludeLightsWhichAreOff = false) = 0;

    /**
     Set a custom light preset on the selected entertainment group
     @param brightness Relative brightness between 0 and 1
     @param x X-coordinate in CIE XY color space
     @param y Y-coordinate in CIE XY color space
     @note this only works when not streaming and a valid group is selected
     */
    virtual void SetGroupPreset(double brightness, double x, double y, bool excludeLightsWhichAreOff = false) = 0;

    /**
     Set the scene of selected entertainment group
     @note this only works when not streaming and a valid group is selected
     */
    virtual void SetGroupScene(const std::string &sceneTag) = 0;

    /**
     Get the time manager
     */
    virtual TimeManagerPtr GetTimeManager() = 0;
};

}  // namespace huestream

#endif  // HUESTREAM_IHUESTREAM_H_
