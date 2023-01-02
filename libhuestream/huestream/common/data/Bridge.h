/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/
/** @file */

#ifndef HUESTREAM_COMMON_DATA_BRIDGE_H_
#define HUESTREAM_COMMON_DATA_BRIDGE_H_

#include "huestream/common/serialize/SerializerHelper.h"
#include "huestream/common/language/IMessageTranslator.h"
#include "huestream/common/data/BridgeSettings.h"
#include "huestream/common/data/Group.h"
#include "support/network/http/HttpRequestError.h"
#include "huestream/common/data/Zone.h"

#include <map>
#include <vector>
#include <string>
#include <sstream>

namespace huestream {

/**
 status of the bridge
 */
enum BridgeStatus {
    BRIDGE_EMPTY,                   ///< empty bridge object
    BRIDGE_NOT_FOUND,               ///< bridge not found
    BRIDGE_INVALID_MODEL,           ///< bridge hardware model does not support streaming
    BRIDGE_NOT_CONNECTABLE,         ///< bridge cannot be connected to because of invalid ip or authorization
    BRIDGE_NOT_CONNECTED,           ///< bridge is not connected
    BRIDGE_INVALID_VERSION,         ///< bridge software version does not support streaming
    BRIDGE_INVALID_CLIENTKEY,       ///< bridge clientkey is invalid
    BRIDGE_NO_GROUP_AVAILABLE,      ///< bridge has no entertainment group configured
    BRIDGE_INVALID_GROUP_SELECTED,  ///< no valid entertainment group is selected yet
    BRIDGE_BUSY,                    ///< bridge is busy streaming by another client
    BRIDGE_READY,                   ///< bridge is ready to stream
    BRIDGE_STREAMING                ///< bridge is streaming
};

/**
 Hue bridge
 */
class Bridge : public Serializable {
 public:
    static constexpr const char* type = "huestream.Bridge";

    static std::map<BridgeStatus, std::string> statusTagMap;

    explicit Bridge(BridgeSettingsPtr bridgeSettings);
    Bridge(std::string id, std::string ip, bool ipValid, BridgeSettingsPtr bridgeSettings);

    virtual ~Bridge() {}

    /**
     empty the bridge object
     */
    void Clear();

    void Serialize(JSONNode *node) const override;

    void Deserialize(JSONNode *node) override;

    std::string SerializeCompact() const;

    void DeserializeCompact(std::string jsonString);

    std::string GetTypeName() const override;

    /**
     get current status of bridge
     */
    virtual BridgeStatus GetStatus() const;

    /**
     get tag or 'string id' of the current bridge status used for user message translation lookup
     */
    virtual std::string GetStatusTag() const;

    /**
     get user message string describing action required from user given current bridge status
     */
    virtual std::string GetUserStatus(MessageTranslatorPtr translator) const;

    /**
     * @return root API url of the bridge (examples: "https://127.0.0.1/api/" or "https://127.0.0.1:4200/api/")
     */
    std::string GetApiRootUrl(bool useClipV2 = false, bool eventing = false) const;

    /**
     get bridge api url
     */
    virtual std::string GetBaseUrl(bool useClipV2 = false, bool eventing = false) const;

    /**
     get bridge url
    */
    virtual std::string GetUrl() const;

    /**
     get bridge small config uri
     */
    virtual std::string GetSmallConfigUrl() const;

    /**
     get uri for selected entertainment group
     */
    virtual std::string GetSelectedGroupUrl() const;

    /**
     get uri for sending action to selected entertainment group
     */
    virtual std::string GetSelectedGroupActionUrl() const;

    /**
     get the uri for getting the app id
    */
    virtual std::string GetAppIdUrl() const;

    /**
     check whether this bridge is currently correctly configured to be streamed to
     */
    virtual bool IsReadyToStream() const;

    /**
     check whether this bridge is currently being streamed to
     */
    virtual bool IsStreaming() const;

    /**
     check if this is an empty bridge object
     */
    virtual bool IsEmpty() const;

    /**
     check if this is a discovered bridge object
     */
    virtual bool IsFound() const;

    /**
     check if this bridge has a valid ip and authorization to connect to
     */
    virtual bool IsConnectable() const;

    /**
      check if this bridge is connected
      */
    virtual bool IsConnected() const;

    /**
     check if this bridge hardware model support streaming
     */
    virtual  bool IsValidModelId() const;

    /**
     check if this bridge software version supports streaming
     */
    virtual bool IsValidApiVersion() const;

    /**
     check if this bridge has a valid client key
     */
    virtual bool IsValidClientKey() const;

    /**
     check if this bridge has ever received valid authorization for streaming
     */
    virtual bool HasEverBeenAuthorizedForStreaming() const;

    /**
     check if this bridge currently has valid authorization for streaming
     */
    virtual bool IsAuthorizedForStreaming() const;

    /**
     select entertainment group if there is only one available, else do nothing
     @return whether an entertainment group has been selected
     */
    virtual bool SelectGroupIfOnlyOneOption();

    /**
     select entertainment group to use for streaming
     @param id Id of the entertainment group to select
     */
    virtual bool SelectGroup(std::string id);

    /**
     get currently selected entertainment group or nullptr if none is selected
     */
    virtual GroupPtr GetGroup() const;

    /**
     get list of lights in the currently selected group
     */
    virtual LightListPtr GetGroupLights() const;

    /**
     get group with specific id or nullptr if none exists
     */
    virtual GroupPtr GetGroupById(const std::string& id) const;

    /**
     delete a group from list of groups in the bridge
     @note only on this bridge object not on actual bridge, mainly internal use
     @param id Id of the entertainment group to select
     */
    virtual void DeleteGroup(std::string id);

    /**
     get groups which another client is currently streaming to
     */
    virtual GroupListPtr GetGroupsOwnedByOtherClient() const;

    /**
     get number of current active streaming sessions on bridge
     */
    virtual int GetCurrentNoStreamingSessions() const;

    /**
     check if this bridge has a group selected
     */
    virtual bool IsGroupSelected() const;

    /**
     check if this bridge has a valid group selected
     */
    virtual bool IsValidGroupSelected() const;

    /**
     check if the proxyNode of the current group is unreachable
     */
    virtual bool IsProxyNodeUnreachable() const;

    /**
     create deep copy of this bridge object
     */
    std::shared_ptr<Bridge> Clone() const;

    /**
     check if bridge support clipv2 api
    */
    bool IsSupportingClipV2() const;

    /**
     get zone with specific id or nullptr if none exists
    */
    virtual ZonePtr GetZoneById(const std::string& id) const;

PROP_DEFINE(Bridge, BridgeSettingsPtr, bridgeSettings, BridgeSettings);
PROP_DEFINE(Bridge, std::string, name, Name);
PROP_DEFINE(Bridge, std::string, modelId, ModelId);
PROP_DEFINE(Bridge, std::string, apiversion, Apiversion);
PROP_DEFINE(Bridge, std::string, swversion, Swversion);
PROP_DEFINE(Bridge, std::string, id, Id);
PROP_DEFINE(Bridge, std::string, ipAddress, IpAddress);
PROP_DEFINE(Bridge, std::string, sslPort, SslPort);
PROP_DEFINE_BOOL(Bridge, bool, isValidIp, IsValidIp);
PROP_DEFINE_BOOL(Bridge, bool, isAuthorized, IsAuthorized);
PROP_DEFINE_BOOL(Bridge, bool, isBusy, IsBusy);
PROP_DEFINE(Bridge, std::string, clientKey, ClientKey);
PROP_DEFINE(Bridge, std::string, user, User);
PROP_DEFINE(Bridge, GroupListPtr, groups, Groups);
PROP_DEFINE(Bridge, std::string, selectedGroup, SelectedGroup);
PROP_DEFINE(Bridge, int, maxNoStreamingSessions, MaxNoStreamingSessions);
PROP_DEFINE(Bridge, std::string, certificate, Certificate);
PROP_DEFINE(Bridge, std::string, appId, AppId);
PROP_DEFINE(Bridge, support::HttpRequestError::ErrorCode, lastHttpErrorCode, LastHttpErrorCode);
PROP_DEFINE(Bridge, int32_t, lastHttpStatusCode, LastHttpStatusCode);
PROP_DEFINE(Bridge, ZoneListPtr, zones, Zones);

 protected:
    virtual void SerializeBase(JSONNode *node) const;

    void DeserializeBase(JSONNode *node);

private:
    void GetStreamApiRootUrl(std::ostringstream& aOSS, bool useClipV2 = false, bool eventing = false) const;
        void OnUserChange();
};

/**
 shared pointer to a huestream::Bridge object
 */
SMART_POINTER_TYPES_FOR(Bridge)

}  // namespace huestream

#endif  // HUESTREAM_COMMON_DATA_BRIDGE_H_
